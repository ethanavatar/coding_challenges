#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>
#include "raylib.h"
#include "raymath.h"
#include <cmath>

#include "common/defer.hpp"
#include "common/math.h"
#include "common/scene.h"

void *starfield_init(void);
void  starfield_update(void  *scene_data, float delta_time);
void  starfield_destroy(void *scene_data);

extern "C"
struct Scene_Functions __declspec(dllexport) get_scene_functions(void) {
    return (struct Scene_Functions) {
        .init    = &starfield_init,
        .update  = &starfield_update,
        .destroy = &starfield_destroy,
    };
}

// @CleanUp: This should be kept at the app-level
const Vector2 SCREEN_SIZE_INITIAL = { .x = 800,  .y = 600  };
const Vector2 CANVAS_SIZE         = { .x = 1200, .y = 1000 };

const size_t STAR_COUNT = 600;

struct Star {
    float x, y, z, last_z;
};

struct Scene_Data {
    bool is_paused;
    struct Star *stars;
};

void *starfield_init(void) {
    struct Scene_Data *data = (struct Scene_Data *) malloc(sizeof(struct Scene_Data));
    memset(data, 0, sizeof(struct Scene_Data));

    struct Star *stars      = (struct Star *) calloc(STAR_COUNT, sizeof(struct Star));
    assert(stars && "Failed to allocate stars");
    memset(stars, 0, STAR_COUNT * sizeof(struct Star));

    for (size_t i = 0; i < STAR_COUNT; ++i) {
        struct Star *star = &stars[i];
        star->x = GetRandomValue(-CANVAS_SIZE.x / 2.f, CANVAS_SIZE.x / 2.f);
        star->y = GetRandomValue(-CANVAS_SIZE.y / 2.f, CANVAS_SIZE.y / 2.f);
        star->z = GetRandomValue(0, CANVAS_SIZE.x / 2.f);
        star->last_z = star->z;
    }

    data->stars = stars;

    return (void *) data;
}

void starfield_destroy(void *scene_data) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;
    DEFER(fprintf(stderr, "Unloaded scene\n"));
    DEFER(free(self));

    free(self->stars);
}

void starfield_update(void *scene_data, float delta_time) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;
    if (IsKeyPressed(KEY_SPACE)) { self->is_paused ^= true; }

    for (size_t i = 0; i < STAR_COUNT; ++i) {
        struct Star *star = &self->stars[i];

        float x = float_remap(star->x / star->z, 0, 1, 0, CANVAS_SIZE.x);
        float y = float_remap(star->y / star->z, 0, 1, 0, CANVAS_SIZE.y);
        float r = float_remap(star->z, 0, CANVAS_SIZE.x / 2.f, 10, 0);

        // https://math.stackexchange.com/questions/3749993/an-equation-for-a-graph-which-resembles-a-hump-of-a-camel-pulse-in-a-string
        // I cant really tell if this is actually working though, lol
        float distance_factor_x;
        float distance_factor_y;
        {
            float a = 3;
            float b = 1;
            float c = 0;
            float d = 1;
            distance_factor_x = a / (1 + b * pow(star->x - c, 2)) + d;
            distance_factor_y = a / (1 + b * pow(star->y - c, 2)) + d;
        }

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

        if (self->is_paused) continue;

        star->last_z = star->z;
        star->z -= 1500 * distance_factor_x * distance_factor_y * delta_time;

        if (star->z < 1) {
            star->x = GetRandomValue(-CANVAS_SIZE.x / 2.f, CANVAS_SIZE.x / 2.f);
            star->y = GetRandomValue(-CANVAS_SIZE.y / 2.f, CANVAS_SIZE.y / 2.f);
            star->z = CANVAS_SIZE.x / 2.f;
            star->last_z = star->z;
        }
    }
}

