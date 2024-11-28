#ifndef E_SCENE_LOADING_H
#define E_SCENE_LOADING_H

#include <assert.h>

#include "WinDef.h"
#include "winbase.h"
#include "libloaderapi.h"

#include "common/scene.h"

void *empty_init(void) { }
void  empty_update(void  *scene_data, float delta_time) { }
void  empty_destroy(void *scene_data) { }

const struct Scene_Functions EMPTY_SCENE_FUNCTIONS = {
    .init    = &empty_init,
    .update  = &empty_update,
    .destroy = &empty_destroy,
};

struct Scene load_scene_from_dll(
    const char *dll_path,
    const char *temp_dll_path,
    const char *pdb_path,
    const char *temp_pdb_path
) {
    struct Scene scene = { 0 };
    scene.last_library_write_time = GetFileModTime(dll_path);

    CopyFileA((LPCSTR) dll_path, (LPCSTR) temp_dll_path, FALSE);
    CopyFileA((LPCSTR) pdb_path, (LPCSTR) temp_pdb_path, FALSE);

    scene.library = LoadLibraryA(temp_dll_path);
    scene.is_valid = true;

    assert(scene.library && "Failed to load scene");
    Scene_Get_Function get_scene_functions = (Scene_Get_Function) GetProcAddress((HMODULE) scene.library, "get_scene_functions");
    scene.functions = get_scene_functions();

    return scene;
}

void unload_scene(struct Scene *scene) {
    if (scene->library) {
        FreeLibrary((HMODULE) scene->library);
        scene->library = NULL;
        scene->functions = EMPTY_SCENE_FUNCTIONS;
    }

    scene->is_valid = false;
}

#endif // E_SCENE_LOADING_H
