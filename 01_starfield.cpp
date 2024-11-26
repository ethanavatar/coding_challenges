#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "raylib.h"

#include "common/defer.hpp"
#include "common/math.h"

const Vector2 SCREEN_SIZE_INITIAL = { .x = 800, .y = 600 };
const Vector2 CANVAS_SIZE         = { .x = 800, .y = 600 };

struct Star {
    float x, y, z, last_z;
};

int main(void) {
    const size_t STAR_COUNT = 800;
    struct Star *stars      = (struct Star *) calloc(STAR_COUNT, sizeof(struct Star));
    memset(stars, 0, STAR_COUNT);
    DEFER(free(stars));

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
        struct Star *star = &stars[i];
        star->x = GetRandomValue(-CANVAS_SIZE.x / 2.f, CANVAS_SIZE.x / 2.f);
        star->y = GetRandomValue(-CANVAS_SIZE.y / 2.f, CANVAS_SIZE.y / 2.f);
        star->z = GetRandomValue(0, CANVAS_SIZE.x / 2.f);
        star->last_z = star->z;
    }

    bool is_paused = false;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        const char *title = TextFormat("starfield - %.2f ms/frame", delta_time * 1'000);
        SetWindowTitle(title);

        if (IsKeyPressed(KEY_SPACE)) { is_paused ^= true; }

        BeginTextureMode(render_target);
            ClearBackground(BLACK);
            BeginMode2D(world_camera);

                for (size_t i = 0; i < STAR_COUNT; ++i) {
                    struct Star *star = &stars[i];

                    float x = float_remap(star->x / star->z, 0, 1, 0, CANVAS_SIZE.x);
                    float y = float_remap(star->y / star->z, 0, 1, 0, CANVAS_SIZE.y);
                    float r = float_remap(star->z, 0, CANVAS_SIZE.x / 2.f, 5, 0);

                    float last_x = float_remap(star->x / star->last_z, 0, 1, 0, CANVAS_SIZE.x);
                    float last_y = float_remap(star->y / star->last_z, 0, 1, 0, CANVAS_SIZE.y);
                    float last_r = float_remap(star->last_z, 0, CANVAS_SIZE.x / 2.f, 5, 0);

                    // Triangles must be drawn counter-clockwise
                    // https://github.com/raysan5/raylib/issues/941
                    
                    if (x > 0 && y > 0) {
                        DrawTriangle(
                            // Left
                            { x - r, y },

                            // Top
                            { x, y - r },
                            { last_x, last_y },
                            WHITE
                        );
                    } else if (x < 0 && y > 0) {
                        DrawTriangle(
                            // Top
                            { x, y - r },

                            // Right
                            { x + r, y },

                            { last_x, last_y },
                            WHITE
                        );
                    } else if (x > 0 && y < 0) {
                        DrawTriangle(
                            // Bottom
                            { x, y + r },

                            // Left
                            { x - r, y },

                            { last_x, last_y },
                            WHITE
                        );
                    } else if (x < 0 && y < 0) {
                        DrawTriangle(
                            // Right
                            { x + r, y },

                            // Bottom
                            { x, y + r },

                            { last_x, last_y },
                            WHITE
                        );
                    }
                    DrawCircle(x, y, r, WHITE);

                    if (is_paused) {
                        continue;
                    }

                    star->last_z = star->z;
                    star->z -= 15;
                    if (star->z < 1) {
                        star->x = GetRandomValue(-CANVAS_SIZE.x / 2.f, CANVAS_SIZE.x / 2.f);
                        star->y = GetRandomValue(-CANVAS_SIZE.y / 2.f, CANVAS_SIZE.y / 2.f);
                        star->z = CANVAS_SIZE.x / 2.f;
                        star->last_z = star->z;
                    }
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
