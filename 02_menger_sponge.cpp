#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "raylib.h"

#include "common/common.h"
#include "common/defer.hpp"
#include "common/scene.h"

void *init(void);
void  update(void  *scene_data, float delta_time);
void  destroy(void *scene_data);

extern "C" struct Scene_Functions __declspec(dllexport) get_scene_functions(void);
struct Scene_Functions get_scene_functions(void) {
    return (struct Scene_Functions) {
        .init    = &init,
        .update  = &update,
        .destroy = &destroy,
    };
}

struct Cube {
    Vector3 position;
    Vector3 size;
};

struct Cube_Array {
    size_t count;
    struct Cube *cubes;
};

const size_t MAX_CUBES = 1024 * 10;

struct Scene_Data {
    Camera3D camera;
    struct Cube_Array active_cubes;
    struct Cube_Array next_cubes;
};

void cube_create(struct Cube_Array *array, Vector3 position, float width) {
    if (array->count >= MAX_CUBES) {
        fprintf(stderr, "array->count: %d\n", array->count);
        fprintf(stderr, "MAX_CUBES:    %d\n", MAX_CUBES);
        assert(false && "Out of bounds");
    }
    struct Cube *c = &array->cubes[array->count++];
    c->position = position;
    c->size     = { width, width, width };
}

void cubes_clear(struct Cube_Array *array) {
    array->count = 0;
}

void cube_subdivide(struct Scene_Data *self, struct Cube cube) {
    for (int x = -1; x < 2; ++x) {
        for (int y = -1; y < 2; ++y) {
            for (int z = -1; z < 2; ++z) {
                int sum = (int) (fabs(x) + fabs(y) + fabs(z));
                if (sum <= 1) continue;

                float w = cube.size.x / 3.f;
                Vector3 pos = {
                    cube.position.x + (x * w),
                    cube.position.y + (y * w),
                    cube.position.z + (z * w)
                };

                cube_create(&self->next_cubes, pos, w);
            }
        }
    }
}

void cubes_subdivide(struct Scene_Data *self) {
    cubes_clear(&self->next_cubes);
    for (size_t cube_index = 0; cube_index < self->active_cubes.count; ++cube_index) {
        struct Cube cube = self->active_cubes.cubes[cube_index];
        cube_subdivide(self, cube);
    }

    memcpy(
        self->active_cubes.cubes,
        self->next_cubes.cubes,
        self->next_cubes.count * sizeof(struct Cube)
    );

    self->active_cubes.count = self->next_cubes.count;
}

void *init(void) {
    struct Scene_Data *self = (struct Scene_Data *) malloc(sizeof(struct Scene_Data));
    assert(self && "failed to allocate scene data");
    memset(self, 0, sizeof(struct Scene_Data));

    {
        self->camera = { };
        self->camera.position = { .x = 10, .y = 10.f, .z = 10.f };
        self->camera.target   = { .x = 0, .y = 0,    .z = 0     };
        self->camera.up       = { .x = 0, .y = 1.f,  .z = 1.f   };
        self->camera.fovy     = 45.f;
        self->camera.projection = CAMERA_PERSPECTIVE;
    }

    self->active_cubes.cubes = (struct Cube *) calloc(MAX_CUBES, sizeof(struct Cube));
    assert(self->active_cubes.cubes && "failed to allocate cubes");

    self->next_cubes.cubes = (struct Cube *) calloc(MAX_CUBES, sizeof(struct Cube));
    assert(self->next_cubes.cubes && "failed to allocate cubes");

    cube_create(&self->active_cubes, { 0, 0, 0 }, 5);
    return (void *) self;
}

void update(void *scene_data, float delta_time) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;

    UpdateCamera(&self->camera, CAMERA_ORBITAL);

    if (IsKeyPressed(KEY_SPACE)) cubes_subdivide(self);

    BeginMode3D(self->camera);
        ClearBackground(BLACK);
        for (size_t cube_index = 0; cube_index < self->active_cubes.count; ++cube_index) {
            struct Cube cube = self->active_cubes.cubes[cube_index];
            DrawCubeV(cube.position, cube.size, RED);
            DrawCubeWiresV(cube.position, cube.size, MAROON);
        }
        DrawGrid(10, 1.0f);
    EndMode3D();
}

void destroy(void *scene_data) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;
    DEFER(fprintf(stderr, "Unloaded scene\n"));
    DEFER(free(self));

    free(self->active_cubes.cubes);
    free(self->next_cubes.cubes);
}

