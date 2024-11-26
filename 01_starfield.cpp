#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "raylib.h"
#include "raymath.h"
#include <cmath>

#include "common/defer.hpp"
#include "common/math.h"

const Vector2 SCREEN_SIZE_INITIAL = { .x = 800, .y = 600 };
const Vector2 CANVAS_SIZE         = { .x = 1200, .y = 1000 };

struct Star {
    float x, y, z, last_z;
};

int main(void) {
    const size_t STAR_COUNT = 600;
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
                    float r = float_remap(star->z, 0, CANVAS_SIZE.x / 2.f, 10, 0);

                    float last_x = float_remap(star->x / star->last_z, 0, 1, 0, CANVAS_SIZE.x);
                    float last_y = float_remap(star->y / star->last_z, 0, 1, 0, CANVAS_SIZE.y);
                    float last_r = float_remap(star->last_z, 0, CANVAS_SIZE.x / 2.f, 5, 0);

                    Vector2 p0 = Vector2Subtract({ last_x, last_y }, {x, y});
                    float   d0 = sqrt(pow(p0.x, 2) + pow(p0.y, 2));
                    Vector2 e1 = Vector2Scale(p0, 1.f / d0);
                    Vector2 e2 = { -p0.y / d0, p0.x / d0 };
                    
                    Vector2 p1 = Vector2Scale(e1, pow(r, 2) / d0);
                    p1 = Vector2Add(p1, Vector2Scale(e2, (r / d0) * sqrt(pow(d0, 2) - pow(r, 2))));
                    p1 = Vector2Add(p1, {x, y});

                    Vector2 p2 = Vector2Scale(e1, pow(r, 2) / d0);
                    p2 = Vector2Subtract(p2, Vector2Scale(e2, (r / d0) * sqrt(pow(d0, 2) - pow(r, 2))));
                    p2 = Vector2Add(p2, {x, y});

                    // Triangles must be drawn counter-clockwise
                    // https://github.com/raysan5/raylib/issues/941
                    DrawTriangle({ last_x, last_y }, p2, p1, WHITE);
                    DrawCircle(x, y, r, WHITE);

                    if (is_paused) {
                        continue;
                    }

                    star->last_z = star->z;
                    star->z -= 25;
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
