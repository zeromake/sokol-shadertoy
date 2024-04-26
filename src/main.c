#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_glue.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
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
} App;


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
    sg_shader shd = sg_make_shader(simple_shader_desc(sg_query_backend()));
    float vertices[] = {
       -1, -1,  // left bottom
        -1, 1,  // left top
        1, 1,   // right top
        -1, -1, // left bottom
        1, 1,   // right top
        1, -1,  // right bottom
    };
    state->elements = sizeof(vertices) / sizeof(float[2]);
    state->bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(vertices),
        .data = SG_RANGE(vertices),
    });
    state->pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .layout = {
            .attrs = {
                [ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT2
            }
        },
    });
    float color = 0xe5 / 255.0f;
    state->pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action=SG_LOADACTION_CLEAR,
            .clear_value={color, color, color, 1.0f},
        }
    };
}

static void frame(void* ptr) {
    App* state = (App*)ptr;
    uint64_t now = getCurrentMilliSecTimestamp();
    uint64_t t = now - state->timestamp;
    uint64_t tt = now - state->prev_timestamp;
    state->prev_timestamp = now;
    state->frag.iResolution[0] = sapp_width();
    state->frag.iResolution[1] = sapp_height();
    state->frag.iTime = (float)t / 1000.0f;
    state->frag.iTimeDelta = (float)tt / 1000.0f;
    state->frag.iFrame = state->iframe;
    sg_begin_pass(&(sg_pass){ .action = state->pass_action, .swapchain = sglue_swapchain() });
    sg_apply_pipeline(state->pip);
    sg_apply_bindings(&state->bind);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_frag, &SG_RANGE(state->frag));
    sg_draw(0, state->elements, 1);
    sg_end_pass();
    sg_commit();
    state->iframe += 1;
}

static void cleanup(void* ptr) {
    sg_shutdown();
}

static void event(const sapp_event* e, void* ptr) {
    App* state = (App*)ptr;
    int index = 0;
    int x = 0;
    int y = 0;
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (e->key_code == SAPP_KEYCODE_ESCAPE) {
                sapp_request_quit();
            }
            break;

        case SAPP_EVENTTYPE_MOUSE_MOVE:
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
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            state->frag.iMouse[2] = e->mouse_button - SAPP_MOUSEBUTTON_LEFT + 1;
            state->frag.iMouse[3] = 1.0;
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            state->frag.iMouse[2] = 0.0;
            state->frag.iMouse[3] = 0.0;
            break;
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
        .height = 600,
        .window_title = "Shadertoy Preview",
        .logger.func = slog_func,
        .high_dpi = true,
        .swap_interval = 1,
    };
}
