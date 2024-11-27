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
    Camera3D camera;
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
        self->camera.position = { .x = 10, .y = 10.f, .z = 10.f };
        self->camera.target   = { .x = 0, .y = 0,    .z = 0 };
        self->camera.up       = { .x = 0, .y = 1.f,  .z = 0 };
        self->camera.fovy     = 45.f;
        self->camera.projection = CAMERA_PERSPECTIVE;
    }

    return (void *) self;
}

void update(void *scene_data, float delta_time) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;

    BeginMode3D(self->camera);
        ClearBackground(BLACK);
        Vector3 cube_position = { 0.f, 0.f, 0.f };
        DrawCube(cube_position, 2.0f, 2.0f, 2.0f, RED);
        DrawCubeWires(cube_position, 2.0f, 2.0f, 2.0f, MAROON);
        DrawGrid(10, 1.0f);
    EndMode3D();
}

void destroy(void *scene_data) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;
    DEFER(fprintf(stderr, "Unloaded scene\n"));
    DEFER(free(self));
}

