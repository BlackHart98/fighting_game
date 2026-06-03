#include "raylib.h"

// Frames will be re-written as a frame generator module
#include "../src/frames.h"

#define WSA_IMPLEMENTATION
#include "../lib/why_so_arena.h"
#define ARRAY_LIST_IMPLEMENTATION
#include "../lib/array_list.h"

#include <stdio.h>
#include <stdlib.h>

#define FLIP_RIGHT      2
#define FLIP_LEFT       1
#define AI_MIN_DISTANCE 70 

enum CHARACTER {
    RYU=0,
};


#define START_POSITION_PLAYER_1 (Vector2){50, 50}
#define START_POSITION_PLAYER_2 (Vector2){200, 50}
#define START_VELOCITY (Vector2){4.0f, 0.0f}


typedef struct animation_t {
    float       frame_speed;
    float       elapsed_time;

    // ...
    int         start_frame;

    // slice_t can hold this
    int         frame_count;
    Rectangle   *frame_rec; // []Rectangle
} animation_t;

typedef struct animation_desc_t {
    int     current_anim;
    int     next_anim;
    float   move_sign;
} animation_desc_t;


animation_desc_t desc[] = {
    [START]          = (animation_desc_t){.current_anim = START, .next_anim = IDLE, },
    [IDLE]           = (animation_desc_t){.current_anim = IDLE, .next_anim = IDLE, },
    [MOVE]           = (animation_desc_t){.current_anim = MOVE, .next_anim = IDLE, },
    [BACKOFF]        = (animation_desc_t){.current_anim = BACKOFF, .next_anim = IDLE, },
    [LEFT_RIGHT_HOOK]= (animation_desc_t){.current_anim = LEFT_RIGHT_HOOK, .next_anim = IDLE, },
    [JAB]            = (animation_desc_t){.current_anim = JAB, .next_anim = IDLE, },
    [LIGHT_KICK]     = (animation_desc_t){.current_anim = LIGHT_KICK, .next_anim = IDLE, },
    [ROUNDHOUSE]     = (animation_desc_t){.current_anim = ROUNDHOUSE, .next_anim = IDLE, },
};


typedef struct player_asset_t {
    char* audio_file;
    char* texture_file;
    char* frames_file;
    int character; 
} player_asset_t;

typedef struct sprite_t{
    Texture2D texture;
    int current_anim_group;
    int prev_anim_group;
    int current_frame_idx;

    // Animations
    int anim_count;
    animation_t *anim;
} sprite_t;


typedef struct player_t {
    sprite_t sprite;
    int pos_idx;
    int vel_idx;
    float health_val;
    char flip; // LEFT | RIGHT
    char player_state; // ACTIVE | INACTIVE
} player_t;


typedef struct versus_t {
    player_t *player;
} versus_t;


void
load_player_assets_fn(arena_allocator_t *allocator, player_t *player, int pos_idx, int vel_idx, Texture2D *texture, slice_t *frame_list, char flip);


void
compute_frame_orientation_fn(animation_t* animation, int current_frame, char flip);

void
update_fn(versus_t *versus, Vector2 *pos, Vector2 *vel, const int screen_width, const int screen_height, float delta);

void
draw_fn(versus_t *versus, Vector2 *pos, Vector2 *vel);

void
player_action_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);

void
ai_action_fn(player_t *player, player_t *opponent, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);


void
draw_character_fn(player_t *player, Vector2 *pos, Vector2 *vel, int current_anim_group);


void
player_tick_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);

 
int
main(void)
{
    const int screen_width = 800;
    const int screen_height = 450;
    player_t player_1 = {0};
    player_t player_2 = {0};
    Vector2 pos[] = {START_POSITION_PLAYER_1, START_POSITION_PLAYER_2};
    Vector2 vel[] = {START_VELOCITY, START_VELOCITY};

    arena_allocator_t gpa = arena_allocator_init_page_default(c_allocator, MB(2));
    InitWindow(screen_width, screen_height, "fighting game");
    {
        Texture2D texture = LoadTexture("assets/Ryu.gif");

        slice_t frames_slice = make_slice(ryu_frames, sizeof(ryu_frames));
        load_player_assets_fn(&gpa, &player_1, 0, 0, &texture, &frames_slice, FLIP_RIGHT);
        load_player_assets_fn(&gpa, &player_2, 1, 1, &texture, &frames_slice, FLIP_LEFT);

        versus_t vs = (versus_t){ .player = (player_t[]){ player_1, player_2 }};
        SetTargetFPS(60); 
        while (!WindowShouldClose()) {
            float delta = GetFrameTime();
            // Update
            update_fn(&vs, pos, vel, screen_width, screen_height, delta);
            // Render
            draw_fn(&vs, pos, vel);       
        }
        UnloadTexture(texture);
    }
    CloseWindow();
    arena_allocator_deinit(&gpa);
    return 0;
}


void
player_action_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height)
{
    // Player Actions
    if (IsKeyDown(KEY_RIGHT) && player->player_state) {
        player->sprite.prev_anim_group = player->sprite.current_anim_group;
        player->sprite.current_anim_group = MOVE;
    }
    if (IsKeyDown(KEY_LEFT) && player->player_state) {
        player->sprite.prev_anim_group = player->sprite.current_anim_group;
        player->sprite.current_anim_group = BACKOFF;

        pos[player->pos_idx].x -= vel[player->vel_idx].x;
        if (pos[player->pos_idx].x <= 50) pos[player->pos_idx].x = 50;
    }
    if (IsKeyPressed(KEY_P) && player->player_state) {
        if (JAB == player->sprite.prev_anim_group){
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            player->sprite.current_anim_group = LEFT_RIGHT_HOOK;
        } else {
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            player->sprite.current_anim_group = JAB;
        }
    }
    if (IsKeyDown(KEY_K) && player->player_state) {
        player->sprite.prev_anim_group = player->sprite.current_anim_group;
        player->sprite.current_anim_group = LIGHT_KICK;
    }
    if (IsKeyDown(KEY_R) && player->player_state) {
        player->sprite.prev_anim_group = player->sprite.current_anim_group;
        player->sprite.current_anim_group = ROUNDHOUSE;
    }
}


void
ai_action_fn(player_t *player, player_t *opponent, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height)
{
    if (player->player_state) {
        float distance = pos[0].x - pos[1].x;
        // move towards player:
        if (-70 > distance) {
            player->sprite.current_anim_group = BACKOFF; // should be move
        }  
        if (70 < distance){
            player->sprite.current_anim_group = MOVE; // should be backoff
        }
    }
}


void
update_fn(versus_t *versus, Vector2 *pos, Vector2 *vel, const int screen_width, const int screen_height, float delta)
{
    // Player Actions
    player_action_fn(&(versus->player[0]), pos, vel, delta, screen_width, screen_height);
    // AI Actions
    ai_action_fn(&(versus->player[1]), &(versus->player[0]), pos, vel, delta, screen_width, screen_height); 
    for (int i = 0; i < 2; i++) {
        player_tick_fn(&(versus->player[i]), pos, vel, delta, screen_width, screen_height);
    }
}


void
draw_fn(versus_t *versus, Vector2 *pos, Vector2 *vel)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        for (int i = 0; i < 2; i++) {
            draw_character_fn(&(versus->player[i]), pos, vel, versus->player[i].sprite.current_anim_group);
        }
    }
    EndDrawing(); 
}



void
compute_frame_orientation_fn(animation_t* animation, int current_frame, char flip)
{
    switch (flip){
        case FLIP_LEFT: {
            if (0 <= animation->frame_rec[current_frame].width) {
                animation->frame_rec[current_frame].width *= -1.0f;
            }
            break;
        }
        case FLIP_RIGHT: {
            if (0 > animation->frame_rec[current_frame].width) {
                animation->frame_rec[current_frame].width *= -1.0f;
            }
            break;
        }
    }

}


void
draw_character_fn(player_t *player, Vector2 *pos, Vector2 *vel, int current_anim_group)
{
    int anim_group = player->sprite.current_anim_group;
    DrawTextureRec(
        player->sprite.texture, 
        player->sprite.anim[anim_group].frame_rec[player->sprite.current_frame_idx], 
        pos[player->pos_idx], WHITE);
}


void
player_tick_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height)
{
    int anim_group = player->sprite.current_anim_group;
    player->sprite.anim[anim_group].elapsed_time += delta;
    float fps = 1 / player->sprite.anim[anim_group].frame_speed;

    if (player->sprite.anim[anim_group].elapsed_time >= fps) {
        // I have to reprogram move and backoff
        if (BACKOFF == anim_group || MOVE == anim_group){

        } else {
            player->sprite.anim[anim_group].elapsed_time = 0.0f;
            player->sprite.current_frame_idx++;
            if (player->sprite.current_frame_idx >= player->sprite.anim[anim_group].frame_count) {
                player->sprite.current_frame_idx = 0;
                player->sprite.current_anim_group = desc[anim_group].next_anim;
            }
        }
    }
    compute_frame_orientation_fn(&(player->sprite.anim[anim_group]), player->sprite.current_frame_idx, player->flip);
}


void
load_player_assets_fn(
    arena_allocator_t *allocator, 
    player_t *player, 
    int pos_idx, 
    int vel_idx, 
    Texture2D *texture, 
    slice_t *frame_list, 
    char flip)
{
    size_t len = (size_t)(frame_list->len_in_bytes / sizeof(frame_rec_slice_t));
    frame_rec_slice_t *ptr = (frame_rec_slice_t *) frame_list->ptr;
    int ret = 0;

    array_list_t anim_arr = array_list_init_capacity(allocator, animation_t, len);
    if (0 != ret) return;
    for (int i = 0; i < (int)len; i++) {
        array_list_t temp_arr = array_list_init_capacity(allocator, Rectangle, ptr[i].len);
        ret = array_list_append_slice_fn(
            allocator, &temp_arr, 
            (slice_t){ 
                .ptr = ptr[i].frame_rec, 
                .len_in_bytes = (sizeof(Rectangle) * ptr[i].len) });
        if (0 != ret) return;
        animation_t anim = (animation_t){
            .elapsed_time = 0.0f,
            .frame_count = (int)ptr[i].len,
            .frame_rec = (Rectangle *) temp_arr.ptr,
            .start_frame = 0,
            .frame_speed = ptr[i].frame_speed,
        };
        ret = array_list_append_item_fn(allocator, &anim_arr, (char *)&anim);
        if (0 != ret) return;
    }
    *player = (player_t){
        .flip = flip,
        .health_val = 0.0f,
        .player_state = 0,
        .pos_idx = pos_idx,
        .vel_idx = vel_idx,
        .sprite = (sprite_t) {
            .anim = (animation_t *) anim_arr.ptr,
            .anim_count = (int)len,
            .current_anim_group = START,
            .prev_anim_group = START,
            .current_frame_idx = 0,
            .texture = *texture,
        },
    };
}