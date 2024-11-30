#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "raylib.h"

#include "common/common.h"
#include "common/defer.hpp"
#include "common/scene.h"
#include "common/math.h"

void *init(void);
void  update(void  *scene_data, float delta_time);
void  destroy(void *scene_data);

const Vector2_Int CELL_SIZE  = { .x = 20, .y = 20 };
const Vector2_Int BOARD_SIZE = { 
    .x = (int) CANVAS_SIZE.x / CELL_SIZE.x,
    .y = (int) CANVAS_SIZE.y / CELL_SIZE.y
};

const size_t SNAKE_MAX_LENGTH = BOARD_SIZE.x * BOARD_SIZE.y;

const float DEATH_ANIMATION_LENGTH = 2.f;

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
    struct Vector2_Int position;
    struct Vector2_Int position_previous;
};

struct Scene_Data {
    Camera2D camera;

    float turn_timer;

    size_t      events_count;
    enum Event *events;

    // @Hack
    bool move_queued;

    bool  is_dying;
    float death_animation_timer;

    struct Vector2_Int food_position;

    // @TODO: Save data between runs
    size_t session_max_length;
    size_t snake_length_max;
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
    self->snake_direction = (enum Direction) GetRandomValue(0, 3);
    self->snake_length = 1;
    self->snake_length_max = self->snake_length;
    self->snake_links[0].position = {
        .x = GetRandomValue(0, BOARD_SIZE.x - 1),
        .y = GetRandomValue(0, BOARD_SIZE.y - 1)
    };
}

void snake_extend(struct Scene_Data *self) {
    struct Snake_Link *old_link = &self->snake_links[self->snake_length - 1];
    struct Snake_Link *new_link = &self->snake_links[self->snake_length++];

    new_link->position = old_link->position_previous;
    self->snake_length_max = self->snake_length;

    if (self->snake_length_max > self->session_max_length) {
        self->session_max_length = self->snake_length_max;
    }
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

    /*
    const char *event_name;
    static_assert(E_EVENTS_COUNT == 5, "Events enum changed");
    switch (event) {
    case E_TURN_UP:    { event_name = "TURN_UP";    } break;
    case E_TURN_DOWN:  { event_name = "TURN_DOWN";  } break;
    case E_TURN_LEFT:  { event_name = "TURN_LEFT";  } break;
    case E_TURN_RIGHT: { event_name = "TURN_RIGHT"; } break;
    case E_EXTEND:     { event_name = "EXTEND";     } break;
    }
    fprintf(stderr, "Enqueue: %s (%d)\n", event_name, event);
    */

    if (self->move_queued) return;

    self->events[self->events_count++] = event;

    switch (event) {
    case E_TURN_UP:
    case E_TURN_DOWN:
    case E_TURN_LEFT:
    case E_TURN_RIGHT: self->move_queued = true;
    }

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

    struct Snake_Link head = self->snake_links[0];
    struct Snake_Link previous_link = head;
    for (size_t i = 1; i < self->snake_length; ++i) {
        self->snake_links[i].position_previous = self->snake_links[i].position;
        self->snake_links[i].position = previous_link.position_previous;
        
        if ((head.position.x == self->snake_links[i].position.x) &&
            (head.position.y == self->snake_links[i].position.y)) {
            
            self->is_dying = true;
            return;
        }

        previous_link = self->snake_links[i];
    }
}

void *init(void) {
    struct Scene_Data *self = (struct Scene_Data *) malloc(sizeof(struct Scene_Data));
    memset(self, 0, sizeof(struct Scene_Data));

    {
        self->camera = { };
        self->camera.target = { .x = -(float) CELL_SIZE.x * 2, .y = -(float) CELL_SIZE.y * 2 };
        self->camera.zoom = 0.9;
    }

    // @CleanUp: MAX_EVENTS
    // @Leak
    self->events = (enum Event *) calloc(24, sizeof(enum Event));

    snake_reset(self);

    // @CleanUp: food_reset
    // @Specificity: It shouldnt be possible to spawn food on a cell that there is currently a snake link
    self->food_position = {
        .x = GetRandomValue(0, BOARD_SIZE.x - 1),
        .y = GetRandomValue(0, BOARD_SIZE.y - 1)
    };

    return (void *) self;
}

void update(void *scene_data, float delta_time) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;

    if (IsKeyPressed(KEY_E))     enqueue_event(self, E_EXTEND);

    if (IsKeyPressed(KEY_UP))    enqueue_event(self, E_TURN_UP);
    if (IsKeyPressed(KEY_DOWN))  enqueue_event(self, E_TURN_DOWN);
    if (IsKeyPressed(KEY_LEFT))  enqueue_event(self, E_TURN_LEFT);
    if (IsKeyPressed(KEY_RIGHT)) enqueue_event(self, E_TURN_RIGHT);

    BeginMode2D(self->camera);
        ClearBackground(DARKGRAY);
        DrawRectangle(0, 0, BOARD_SIZE.x * CELL_SIZE.x, BOARD_SIZE.y * CELL_SIZE.y, BLACK);

        // @TODO: Seperate UI camera
        const char *score_text = TextFormat("Score: %zu", self->snake_length);
        DrawText(score_text, -30, -30, 25, WHITE);

        const char *high_score_text = TextFormat("High Score: %zu", self->session_max_length);
        DrawText(high_score_text, 100, -30, 25, WHITE);

        snake_draw(self);

        DrawRectangle(
            self->food_position.x * CELL_SIZE.x,
            self->food_position.y * CELL_SIZE.y,
            CELL_SIZE.x,
            CELL_SIZE.y,
            RED
        );
    EndMode2D();

    // @CleanUp: vector2_equal
    if ((self->snake_links[0].position.x == self->food_position.x) &&
        (self->snake_links[0].position.y == self->food_position.y)) {

        enqueue_event(self, E_EXTEND);

        // @CleanUp: food_reset
        // @Specificity: It shouldnt be possible to spawn food on a cell that there is currently a snake link
        self->food_position = {
            .x = GetRandomValue(0, BOARD_SIZE.x - 1),
            .y = GetRandomValue(0, BOARD_SIZE.y - 1)
        };
    }

    if (self->is_dying) {

        size_t kill_link_count = floor(
            remap(0.f, DEATH_ANIMATION_LENGTH, 0.f, (float) self->snake_length_max, self->death_animation_timer)
        );
        fprintf(stderr, "kill_link_count: %zu\n", kill_link_count);


        if (self->death_animation_timer < DEATH_ANIMATION_LENGTH) {
            self->snake_length = self->snake_length_max - kill_link_count;

            self->death_animation_timer += delta_time;
            fprintf(stderr, "death_animation_timer: %.2f\n", self->death_animation_timer);

        } else {
            self->is_dying = false;
            self->death_animation_timer = 0;
            snake_reset(self);
        }

    } else {
        if (self->turn_timer >= 0.0625f) {
            end_turn(self);
            self->turn_timer = 0;
        }

        self->turn_timer += delta_time;
    }
}

void destroy(void *scene_data) {
    struct Scene_Data *self = (struct Scene_Data *) scene_data;
    DEFER(fprintf(stderr, "Unloaded scene\n"));
    DEFER(free(self));
}

