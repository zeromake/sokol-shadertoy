//------------------------------------------------------------------------------
//  dbgui.cc
//  Implementation file for the generic debug UI overlay, using
//  the sokol_imgui.h utility header which implements the Dear ImGui
//  glue code.
//------------------------------------------------------------------------------
#include <sokol_gfx.h>
#include <sokol_app.h>
#include <sokol_log.h>
#include <math.h>
#include <imgui/imgui.h>
#include <imgui/misc/freetype/imgui_freetype.h>
#include <imgui/misc/fonts/DroidSans.h>
#define SOKOL_IMGUI_IMPL
#include "util/sokol_imgui.h"
#define SOKOL_GFX_IMGUI_IMPL
#include "util/sokol_gfx_imgui.h"

extern "C" {

static sgimgui_t sg_imgui;

static void setupScale(float scale)
{
    ImGuiIO& io = ImGui::GetIO();
    static const ImWchar rangesBasic[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x03BC, 0x03BC, // micro
        0x03C3, 0x03C3, // small sigma
        0x2013, 0x2013, // en dash
        0x2264, 0x2264, // less-than or equal to
        0,
    };
    ImFontConfig configBasic;
    configBasic.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryCompressedTTF(
        DroidSans_compressed_data,
        DroidSans_compressed_size,
        round( 15.0f * scale ),
        &configBasic,
        rangesBasic
    );
    io.FontGlobalScale = 1.0f / scale;
}

void __dbgui_setup(int sample_count) {
    // setup debug inspection header(s)
    const sgimgui_desc_t desc = { };
    sgimgui_init(&sg_imgui, &desc);

    // setup the sokol-imgui utility header
    simgui_desc_t simgui_desc = { };
    simgui_desc.sample_count = sample_count;
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
    if (sapp_dpi_scale() != 1.0f)
        setupScale(sapp_dpi_scale());
}

void __dbgui_shutdown(void) {
    sgimgui_discard(&sg_imgui);
    simgui_shutdown();
}

void __dbgui_draw(void) {
    simgui_new_frame({ sapp_width(), sapp_height(), sapp_frame_duration(), sapp_dpi_scale() });
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("sokol-gfx")) {
            ImGui::MenuItem("Capabilities", 0, &sg_imgui.caps_window.open);
            ImGui::MenuItem("Buffers", 0, &sg_imgui.buffer_window.open);
            ImGui::MenuItem("Images", 0, &sg_imgui.image_window.open);
            ImGui::MenuItem("Samplers", 0, &sg_imgui.sampler_window.open);
            ImGui::MenuItem("Shaders", 0, &sg_imgui.shader_window.open);
            ImGui::MenuItem("Pipelines", 0, &sg_imgui.pipeline_window.open);
            ImGui::MenuItem("Frames", 0, &sg_imgui.frame_stats_window.open);
            ImGui::MenuItem("Calls", 0, &sg_imgui.capture_window.open);
            ImGui::MenuItem("Attachs", 0, &sg_imgui.attachments_window.open);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    sgimgui_draw(&sg_imgui);
    simgui_render();
}

void __dbgui_event(const sapp_event* e) {
    simgui_handle_event(e);
}

bool __dbgui_event_with_retval(const sapp_event* e) {
    return simgui_handle_event(e);
}

} // extern "C"
