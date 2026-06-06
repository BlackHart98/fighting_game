#ifndef LOAD_GAME_ASSETS_H
#define LOAD_GAME_ASSETS_H
#include "raylib.h"
#include <stdlib.h>

#include "../src/frames.h"
#include "../lib/why_so_arena.h"
#include "../lib/array_list.h"

#define FLIP_RIGHT      2
#define FLIP_LEFT       1
#define AI_MIN_DISTANCE 70 

enum CHARACTER {
    RYU=0,
};

#define ABS(x) (0 > x)? (-1 * x) : x 
#define START_POSITION_PLAYER_1 (Vector2){50, 50}
#define START_POSITION_PLAYER_2 (Vector2){200, 50}
#define START_POSITION_PLAYER_3 (Vector2){0, 400}
#define START_VELOCITY (Vector2){4.0f, 0.0f}


typedef struct animation_t {
    float       frame_speed;
    float       elapsed_time;

    // ...
    int         start_frame;
    int         current_frame_idx;

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

    // Animations
    int anim_count;
    animation_t *anim;
} sprite_t;


typedef struct player_t {
    sprite_t sprite;
    int pos_idx;
    int vel_idx;
    float health_val;
    char flip; // FLIP_LEFT | FLIP_RIGHT
    char player_state; // ACTIVE | INACTIVE
} player_t;


typedef struct ground_t {
#ifdef HAS_TEXTURE
    sprite_t sprite;
#else
    int   height;
    int   width;
#endif
    int pos_idx;
} ground_t;

typedef struct game_object_t {
    player_t *player;
    ground_t ground;
} game_object_t;


void
load_player_assets_fn(
    arena_allocator_t *allocator, 
    player_t *player, 
    int pos_idx, 
    int vel_idx, 
    Texture2D *texture, 
    slice_t *frame_list, 
    char flip);

void
load_ground_assets_fn(ground_t *ground, int pos_idx, const int screen_width, const int screen_height);




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
            .current_frame_idx = 0,
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
            .texture = *texture,
        },
    };
}


void
load_ground_assets_fn(ground_t *ground, int pos_idx, const int screen_width, const int screen_height)
{
    *ground = (ground_t){
        .pos_idx = pos_idx,
        .height = 50,
        .width = screen_width,
    };
}
#endif