#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "raylib.h"

#include "common/common.h"
#include "common/defer.hpp"
#include "common/scene.h"

struct Vector2Int { int x, y; };

void *init(void);
void  update(void  *scene_data, float delta_time);
void  destroy(void *scene_data);

const Vector2Int CELL_SIZE  = { .x = 20, .y = 20 };
const Vector2Int BOARD_SIZE = { 
    .x = (int) CANVAS_SIZE.x / CELL_SIZE.x,
    .y = (int) CANVAS_SIZE.y / CELL_SIZE.y
};

const size_t SNAKE_MAX_LENGTH = BOARD_SIZE.x * BOARD_SIZE.y;

enum Event {
    E_TURN_UP,   E_TURN_DOWN,
    E_TURN_LEFT, E_TURN_RIGHT,
    E_EXTEND,
    E_EVENTS_COUNT,
};

enum Direction {
    DIRECTION_UP,   DIRECTION_DOWN,
    DIRECTION_LEFT, DIRECTION_RIGHT,
};

struct Snake_Link {
    struct Vector2Int position;
    struct Vector2Int position_previous;
};

struct Scene_Data {
    Camera2D camera;

    float turn_timer;

    size_t     events_count;
    enum Event *events;

    bool move_queued;

    size_t snake_length;
    struct Snake_Link *snake_links;
    enum   Direction   snake_direction;
};

extern "C" struct Scene_Functions __declspec(dllexport) get_scene_functions(void);
struct Scene_Functions get_scene_functions(void) {
    return (struct Scene_Functions) {
        .init    = &init,
        .update  = &update,
        .destroy = &destroy,
    };
}

void snake_reset(struct Scene_Data *self) {
    // @Leak
    self->snake_links = (struct Snake_Link *) calloc(SNAKE_MAX_LENGTH, sizeof(struct Snake_Link));
    self->snake_length = 1;
    self->snake_links[0].position = {
        .x = GetRandomValue(0, BOARD_SIZE.x),
        .y = GetRandomValue(0, BOARD_SIZE.y)
    };
}

void snake_extend(struct Scene_Data *self) {
    struct Snake_Link *old_link = &self->snake_links[self->snake_length - 1];
    struct Snake_Link *new_link = &self->snake_links[self->snake_length++];

    new_link->position = old_link->position_previous;
    //new_link->position_previous = new_link->position;

    /*
    fprintf(stderr, "old position: %d, %d\n", old_link->position.x,          old_link->position.y);
    fprintf(stderr, "old previous: %d, %d\n", old_link->position_previous.x, old_link->position_previous.y);
    fprintf(stderr, "new position: %d, %d\n", new_link->position.x,          new_link->position.y);
    fprintf(stderr, "new previous: %d, %d\n", new_link->position_previous.x, new_link->position_previous.y);
    */
}

void snake_draw(struct Scene_Data *self) {
    for (size_t i = 0; i < self->snake_length; ++i) {
        DrawRectangle(
            self->snake_links[i].position.x * CELL_SIZE.x,
            self->snake_links[i].position.y * CELL_SIZE.y,
            CELL_SIZE.x,
            CELL_SIZE.y,
            WHITE
        );
    }
}

void enqueue_event(struct Scene_Data *self, enum Event event) {

    const char *event_name;
    static_assert(E_EVENTS_COUNT == 5, "Events enum changed");
    switch (event) {
    case E_TURN_UP:    { event_name = "TURN_UP";    } break;
    case E_TURN_DOWN:  { event_name = "TURN_DOWN";  } break;
    case E_TURN_LEFT:  { event_name = "TURN_LEFT";  } break;
    case E_TURN_RIGHT: { event_name = "TURN_RIGHT"; } break;
    case E_EXTEND:     { event_name = "EXTEND";     } break;
    }

    if (self->move_queued) return;

    self->events[self->events_count++] = event;

    switch (event) {
    case E_TURN_UP:
    case E_TURN_DOWN:
    case E_TURN_LEFT:
    case E_TURN_RIGHT: self->move_queued = true;
    }

    fprintf(stderr, "Enqueue: %s (%d)\n", event_name, event);
}

void end_turn(struct Scene_Data *self) {

    self->snake_links[0].position_previous = self->snake_links[0].position;

    switch (self->snake_direction) {
    case DIRECTION_UP:    { self->snake_links[0].position.y -= 1; } break;
    case DIRECTION_DOWN:  { self->snake_links[0].position.y += 1; } break;
    case DIRECTION_LEFT:  { self->snake_links[0].position.x -= 1; } break;
    case DIRECTION_RIGHT: { self->snake_links[0].position.x += 1; } break;
    }

    // @HACK: This is using BOARD_SIZE.x/y - 1 and x/y == BOARD_SIZE.x/y because of an off-by-one error somewhere else
    if (self->snake_links[0].position.x < 0) self->snake_links[0].position.x = BOARD_SIZE.x - 1;
    if (self->snake_links[0].position.y < 0) self->snake_links[0].position.y = BOARD_SIZE.y - 1;
    if (self->snake_links[0].position.x == BOARD_SIZE.x) self->snake_links[0].position.x = 0;
    if (self->snake_links[0].position.y == BOARD_SIZE.y) self->snake_links[0].position.y = 0;


    //fprintf(stderr, "position: %d, %d\n", self->snake_links[0].position.x, self->snake_links[0].position.y);
    //fprintf(stderr, "previous: %d, %d\n", self->snake_links[0].position_previous.x, self->snake_links[0].position_previous.y);

    for (size_t i = 0; i < self->events_count; ++i) {
        switch (self->events[i]) {
        case E_EXTEND: { snake_extend(self); } break;

        case E_TURN_UP: {
            if (self->snake_direction == DIRECTION_DOWN) break;
            self->snake_direction = DIRECTION_UP;
        } break;

        case E_TURN_DOWN: {
            if (self->snake_direction == DIRECTION_UP) break;
            self->snake_direction = DIRECTION_DOWN;
        } break;

        case E_TURN_LEFT: {
            if (self->snake_direction == DIRECTION_RIGHT) break;
            self->snake_direction = DIRECTION_LEFT;
        } break;

        case E_TURN_RIGHT: {
            if (self->snake_direction == DIRECTION_LEFT) break;
            self->snake_direction = DIRECTION_RIGHT;
        } break;
        }
    }

    self->events_count = 0;
    self->move_queued = false;

    struct Snake_Link previous_link = self->snake_links[0];
    for (size_t i = 1; i < self->snake_length; ++i) {
        self->snake_links[i].position_previous = self->snake_links[i].position;
        self->snake_links[i].position = previous_link.position_previous;
        previous_link = self->snake_links[i];
    }
    
}

void *init(void) {
    struct Scene_Data *self = (struct Scene_Data *) malloc(sizeof(struct Scene_Data));
    memset(self, 0, sizeof(struct Scene_Data));

    {
        self->camera = { };
        self->camera.zoom = 1;
        //self->camera.target = { .x = -CANVAS_SIZE.x / 2.f, .y = -CANVAS_SIZE.y / 2.f };
    }

    // @CleanUp: MAX_EVENTS
    // @Leak
    self->events = (enum Event *) calloc(24, sizeof(enum Event));

    snake_reset(self);

    return (void *) self;
}

void update(void *scene_data, float delta_time) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;

    if (IsKeyPressed(KEY_R))     snake_reset(self);
    if (IsKeyPressed(KEY_E))     enqueue_event(self, E_EXTEND);
    if (IsKeyPressed(KEY_UP))    enqueue_event(self, E_TURN_UP);
    if (IsKeyPressed(KEY_DOWN))  enqueue_event(self, E_TURN_DOWN);
    if (IsKeyPressed(KEY_LEFT))  enqueue_event(self, E_TURN_LEFT);
    if (IsKeyPressed(KEY_RIGHT)) enqueue_event(self, E_TURN_RIGHT);
    //if (IsKeyPressed(KEY_SPACE)) end_turn(self);

    BeginMode2D(self->camera);
        ClearBackground(BLACK);
        snake_draw(self);
    EndMode2D();

    if (self->turn_timer >= 0.0625f) {
        end_turn(self);
        self->turn_timer = 0;
    }

    self->turn_timer += delta_time;
}

void destroy(void *scene_data) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;
    DEFER(fprintf(stderr, "Unloaded scene\n"));
    DEFER(free(self));
}

