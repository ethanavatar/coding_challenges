#include <cstdio>
#include "raylib.h"

#include "common/common.h"
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

void *init(void) {
    return NULL;
}

void update(void *scene_data, float delta_time) {
    DrawTriangle(
        {  0.f, -CANVAS_SIZE.y / 4.f },
        { -CANVAS_SIZE.x / 4.f, CANVAS_SIZE.y / 4.f },
        {  CANVAS_SIZE.x / 4.f, CANVAS_SIZE.y / 4.f },
        WHITE
    );
}

void destroy(void *scene_data) { }
