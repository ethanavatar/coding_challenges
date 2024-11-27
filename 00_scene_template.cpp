#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "raylib.h"

#include "common/common.h"
#include "common/defer.hpp"
#include "common/scene.h"

void *init(void);
void  update(void  *scene_data, float delta_time);
void  destroy(void *scene_data);

struct Scene_Data {
    Camera2D camera;
};

extern "C" struct Scene_Functions __declspec(dllexport) get_scene_functions(void);
struct Scene_Functions get_scene_functions(void) {
    return (struct Scene_Functions) {
        .init    = &init,
        .update  = &update,
        .destroy = &destroy,
    };
}

void *init(void) {
    struct Scene_Data *self = (struct Scene_Data *) malloc(sizeof(struct Scene_Data));
    memset(self, 0, sizeof(struct Scene_Data));

    {
        self->camera = { };
        self->camera.zoom = 1;
        self->camera.target = { .x = -CANVAS_SIZE.x / 2.f, .y = -CANVAS_SIZE.y / 2.f };
    }

    return (void *) self;
}

void update(void *scene_data, float delta_time) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;

    BeginMode2D(self->camera);
        ClearBackground(BLACK);
        DrawTriangle(
            {  0.f, -CANVAS_SIZE.y / 4.f },
            { -CANVAS_SIZE.x / 4.f, CANVAS_SIZE.y / 4.f },
            {  CANVAS_SIZE.x / 4.f, CANVAS_SIZE.y / 4.f },
            WHITE
        );
    EndMode2D();
}

void destroy(void *scene_data) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;
    DEFER(fprintf(stderr, "Unloaded scene\n"));
    DEFER(free(self));
}

