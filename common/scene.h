#ifndef E_SCENE_H
#define E_SCENE_H

typedef void *(*Scene_Init_Function)    (void);
typedef void  (*Scene_Update_Function)  (void *, float);
typedef void  (*Scene_Destroy_Function) (void *);

struct Scene_Functions {
    Scene_Init_Function    init;
    Scene_Update_Function  update;
    Scene_Destroy_Function destroy;
};

typedef struct Scene_Functions Scene_Functions_T;
typedef Scene_Functions_T (*Scene_Get_Function)(void);

struct Scene {
    void *library;
    long  last_library_write_time;
    bool  is_valid;

    void *scene_data;
    struct Scene_Functions functions;
};

#endif // E_SCENE_H
