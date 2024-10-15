#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_glue.h>
#include <sokol_fetch.h>
#include <stb/stb_image.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dbgui.h"
#include "shadertoy.glsl.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <time.h>
uint64_t getCurrentMilliSecTimestamp(){
    FILETIME file_time;
    GetSystemTimeAsFileTime(&file_time);
    uint64_t time = ((uint64_t)file_time.dwLowDateTime) + ((uint64_t)file_time.dwHighDateTime << 32);
    uint64_t EPOCH = ((uint64_t)116444736000000000ULL);
    return (uint64_t)((time - EPOCH) / 10000LL);
}
#else
#include <sys/time.h>
#include <unistd.h>
uint64_t getCurrentMilliSecTimestamp(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

typedef struct App {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    uint64_t timestamp;
    uint64_t prev_timestamp;
    int iframe;
    int elements;
    frag_t frag;
    uint8_t file_buffer[1024 * 1024 * 2];
} App;

static void fetch_callback(const sfetch_response_t*);

static void init(void* ptr) {
    App* state = (App*)ptr;
    memset(&state->frag, 0, sizeof(frag_t));
    state->timestamp = getCurrentMilliSecTimestamp();
    state->frag.iSampleRate = 44100.0f;
    state->frag.iSampleRate = 60.0f;
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    // 初始化 sfetch_setup
    sfetch_setup(&(sfetch_desc_t){
        .max_requests = 2,
        .num_channels = 1,
        .num_lanes = 1,
        .logger.func = slog_func,
    });

    __dbgui_setup(1);
    // stb 设置 y 轴翻转(改为在 shader 中翻转)
    // stbi_set_flip_vertically_on_load(true);

    sg_shader shd = sg_make_shader(simple_shader_desc(sg_query_backend()));
    // 必须是三角型，否则无法显示
    // 顶点坐标
    float vertices[] = {
        1, 1,   // RT
        1, -1,  // RB
        -1, -1, // LB
        -1, 1,  // LT
    };
    // 顶点索引
    uint16_t indices[] = {
        0, 1, 3, // RT -> RB -> LT
        1, 2, 3, // RB -> LB -> LT
    };
    // 需要绘制的顶点次数
    state->elements = sizeof(indices) / sizeof(uint16_t);
    // 绑定顶点数据
    state->bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(vertices),
        .data = SG_RANGE(vertices),
        .label = "vertices",
    });
    // 绑定索引数据
    state->bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .data = SG_RANGE(indices),
        .label = "indices",
    });
    // 创建管线
    state->pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        // 定义索引类型
        .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .attrs = {
                // 设置顶点坐标输入类型
                [ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT2
            }
        },
        .label = "image-pipeline",
    });
    // 设置清屏颜色
    float color = 0xe5 / 255.0f;
    state->pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action=SG_LOADACTION_CLEAR,
            .clear_value={color, color, color, 1.0f},
        },
    };

    // 初始化纹理
    state->bind.fs.images[SLOT__iChannel0] = sg_alloc_image();
    state->bind.fs.samplers[SLOT__smp0] = sg_alloc_sampler();
    sg_init_sampler(state->bind.fs.samplers[SLOT__smp0], &(sg_sampler_desc){
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .compare = SG_COMPAREFUNC_NEVER,
        .label = "smp0",
    });
    sfetch_send(&(sfetch_request_t){
        .path = "src/resource/renderpass0-channel0.png",
        .callback = fetch_callback,
        .buffer = SFETCH_RANGE(state->file_buffer),
        .user_data = SFETCH_RANGE(state),
    });
}

static void fetch_callback(const sfetch_response_t* response) {
    App *state = *(App**)response->user_data;
    if (response->fetched) {
        int img_width, img_height, num_channels;
        const int desired_channels = 4;
        // 使用 stb
        stbi_uc* pixels = stbi_load_from_memory(
            response->data.ptr,
            (int)response->data.size,
            &img_width, &img_height,
            &num_channels, desired_channels);
        if (pixels) {
            /* initialize the sokol-gfx texture */
            sg_init_image(state->bind.fs.images[SLOT__iChannel0], &(sg_image_desc){
                .width = img_width,
                .height = img_height,
                /* set pixel_format to RGBA8 for WebGL */
                .pixel_format = SG_PIXELFORMAT_RGBA8,
                .usage = SG_USAGE_IMMUTABLE,
                .data.subimage[0][0] = {
                    .ptr = pixels,
                    .size = img_width * img_height * 4,
                },
                .label = "iChannel0",
            });
            stbi_image_free(pixels);
        }
    } else if (response->failed) {
        // if loading the file failed, set clear color to red
        state->pass_action = (sg_pass_action) {
            .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 1.0f, 0.0f, 0.0f, 1.0f } }
        };
    }
}

static void frame(void* ptr) {
    sfetch_dowork();
    App* state = (App*)ptr;

    // 更新 uniform 变量
    uint64_t now = getCurrentMilliSecTimestamp();
    uint64_t t = now - state->timestamp;
    uint64_t tt = now - state->prev_timestamp;
    state->prev_timestamp = now;
    // 更新画布大小
    state->frag.iResolution[0] = sapp_width();
    state->frag.iResolution[1] = sapp_height();
    // 更新时间
    state->frag.iTime = (float)t / 1000.0f;
    state->frag.iTimeDelta = (float)tt / 1000.0f;
    // 更新绘制次数
    state->frag.iFrame = state->iframe;

    // 绑定绘制目标，并自动清屏
    sg_begin_pass(&(sg_pass){ .action = state->pass_action, .swapchain = sglue_swapchain() });
    // 绑定管线
    sg_apply_pipeline(state->pip);
    // 设置顶点数据到管线
    sg_apply_bindings(&state->bind);
    // 设置 fs 的 uniform 变量到管线
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_frag, &SG_RANGE(state->frag));
    // 根据顶点数量进行绘制
    sg_draw(0, state->elements, 1);
    // 解除管线绑定
    __dbgui_draw();
    sg_end_pass();
    // 提交绘制到目标
    sg_commit();
    state->iframe += 1;
}

static void cleanup(void* ptr) {
    __dbgui_shutdown();
    sg_shutdown();
    sfetch_shutdown();
}

static void event(const sapp_event* e, void* ptr) {
    // dbgui 接管事件
    if (__dbgui_event_with_retval(e)) return;

    // dbgui 不使用的事件才传递到这里
    App* state = (App*)ptr;
    int index = 0;
    int x = 0;
    int y = 0;
    if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (e->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_request_quit();
        }
    } else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
        x = e->mouse_x;
        y = e->mouse_y;
        y = sapp_height() - y;
        // 苹果系统需要缩放鼠标坐标
#ifdef __APPLE__
        x = (float)x / sapp_dpi_scale();
        y = (float)y / sapp_dpi_scale();
#endif
        state->frag.iMouse[0] = x;
        state->frag.iMouse[1] = y;
    } else if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN) {
        state->frag.iMouse[2] = e->mouse_button - SAPP_MOUSEBUTTON_LEFT + 1;
        state->frag.iMouse[3] = 1.0;
    } else if (e->type == SAPP_EVENTTYPE_MOUSE_UP) {
        state->frag.iMouse[2] = 0.0;
        state->frag.iMouse[3] = 0.0;
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    App* app = (App*)malloc(sizeof(App));
    memset(app, 0, sizeof(App));
    return (sapp_desc){
        .init_userdata_cb = init,
        .frame_userdata_cb = frame,
        .cleanup_userdata_cb = cleanup,
        .event_userdata_cb = event,
        .user_data = app,
        .width = 800,
        .height = 450,
        .window_title = "Shadertoy Preview",
        .logger.func = slog_func,
        .high_dpi = true,
        .swap_interval = 1,
    };
}
