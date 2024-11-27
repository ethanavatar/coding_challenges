#include <ctime>
#include <cstdio>

#include "raylib.h"

#include "common/common.h"
#include "common/defer.hpp"
#include "common/scene_loading.h"

int main(void) {
    SetRandomSeed(time(NULL));
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_SIZE_INITIAL.x, SCREEN_SIZE_INITIAL.y, "starfield");
    DEFER(CloseWindow());

    SetTargetFPS(60);

    RenderTexture2D render_target = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
    DEFER(UnloadRenderTexture(render_target));

    Vector2 dpi_scale   = GetWindowScaleDPI();
    float window_width  = GetRenderWidth()  / dpi_scale.x;
    float window_height = GetRenderHeight() / dpi_scale.y;
    float window_scale  = (float) window_height / CANVAS_SIZE.y;

    struct Scene current_scene_info = { 0 };
    //current_scene_info = load_scene_from_dll("bin/01_starfield.dll", "bin/01_starfield_loaded.dll");
    current_scene_info = load_scene_from_dll("bin/02_menger_sponge.dll", "bin/02_menger_sponge_loaded.dll");
    struct Scene_Functions current_scene = current_scene_info.functions;

    void *scene_data = current_scene.init();

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        const char *title = TextFormat("coding challenges - %.2f ms/frame", delta_time * 1'000);
        SetWindowTitle(title);

        BeginTextureMode(render_target);
            current_scene.update(scene_data, delta_time);
        EndTextureMode();

        if (IsWindowResized()) {
            window_width  = GetRenderWidth()  / dpi_scale.x;
            window_height = GetRenderHeight() / dpi_scale.y;
            window_scale  = window_height / CANVAS_SIZE.y;
        }

        BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawTexturePro(
                render_target.texture,
                { 0.f, 0.f, (float) render_target.texture.width, -(float) render_target.texture.height },
                {
                    window_width / 2.f - (render_target.texture.width * window_scale) / 2.f,
                    0,
                    render_target.texture.width  * window_scale,
                    render_target.texture.height * window_scale
                },
                { 0.0f, 0.0f }, 0.0f, WHITE
            );
        EndDrawing();
    }

    current_scene.destroy(scene_data);
    return 0;
}
