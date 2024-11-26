#include <cstdio>
#include <ctime>
#include "raylib.h"
#include "common/defer.hpp"

const Vector2 SCREEN_SIZE_INITIAL = { .x = 800, .y = 600 };
const Vector2 CANVAS_SIZE         = { .x = 800, .y = 600 };

const size_t STAR_COUNT   = 200;
Vector3 stars[STAR_COUNT] = { 0 };

int main(void) {
    SetRandomSeed(time(NULL));
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_SIZE_INITIAL.x, SCREEN_SIZE_INITIAL.y, "starfield");
    DEFER(CloseWindow());

    SetTargetFPS(60);

    Camera2D world_camera = { };
    world_camera.zoom = 1;
    world_camera.target = { .x = -CANVAS_SIZE.x / 2.f, .y = -CANVAS_SIZE.y / 2.f };

    RenderTexture2D render_target = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
    DEFER(UnloadRenderTexture(render_target));

    Vector2 dpi_scale   = GetWindowScaleDPI();
    float window_width  = GetRenderWidth()  / dpi_scale.x;
    float window_height = GetRenderHeight() / dpi_scale.y;
    float window_scale  = (float) window_height / CANVAS_SIZE.y;

    for (size_t i = 0; i < STAR_COUNT; ++i) {
        stars[i].x = GetRandomValue(-CANVAS_SIZE.x / 2.f, CANVAS_SIZE.x / 2.f);
        stars[i].y = GetRandomValue(-CANVAS_SIZE.y / 2.f, CANVAS_SIZE.y / 2.f);
        stars[i].z = GetRandomValue(-CANVAS_SIZE.x / 2.f, CANVAS_SIZE.x / 2.f);
    }

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        const char *title = TextFormat("starfield - %.2f ms/frame", delta_time * 1'000);
        SetWindowTitle(title);

        BeginTextureMode(render_target);
            ClearBackground(BLACK);
            BeginMode2D(world_camera);
                for (size_t i = 0; i < STAR_COUNT; ++i) {
                    Vector3 *star = &stars[i];
                    DrawCircle(star->x / star->z, star->y / star->z, 1.f, WHITE);
                    star->z -= 1;
                }
            EndMode2D();
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

    return 0;
}
