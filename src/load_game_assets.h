#ifndef LOAD_GAME_ASSETS_H
#define LOAD_GAME_ASSETS_H
#include "raylib.h"
#include <stdlib.h>

#include "../src/frames.h"
#include "../lib/why_so_arena.h"
#include "../lib/array_list.h"
#include "../src/physics.h"

#define FLIP_RIGHT      2
#define FLIP_LEFT       1
#define AI_MIN_DISTANCE 70 

enum CHARACTER {
    RYU=0,
};

enum INPUT_TYPE {
    PLAYER, AI
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
    int character;
    int player_input;
    int input_type;
    int pos_idx;
    float health_val;
    char flip; // FLIP_LEFT | FLIP_RIGHT
    char player_state; // ACTIVE | INACTIVE
} player_t;


typedef struct player_selector_t {
    int character;
    int player_input;
} player_selector_t;


typedef struct enviroment_selector_t {
    int env_1;
} enviroment_selector_t;


typedef struct selector_t {
    player_selector_t player_1;
    player_selector_t player_2;
    struct {
        int env;
    }  environment;
} selector_t;


typedef struct ground_t {
#ifdef HAS_TEXTURE
    sprite_t sprite;
#endif
    int pos_idx;
} ground_t;

typedef struct game_object_t {
    player_t player_1;
    player_t player_2;
    ground_t ground;
} game_object_t;


void
load_player_assets_fn(
    arena_allocator_t *allocator, 
    player_t *player, 
    player_selector_t *selector,
    position_soa_t *position,
    int pos_idx,
    Texture2D *texture, 
    slice_t *frame_list, 
    char flip);

void
load_ground_assets_fn(ground_t *ground, int pos_idx);


void
load_game_assets_fn(
    arena_allocator_t *allocator, 
    game_object_t *game_obj, 
    position_soa_t *position,
    const selector_t *selector,
    const int screen_width, 
    const int screen_height);



void
load_player_assets_fn(
    arena_allocator_t *allocator, 
    player_t *player, 
    player_selector_t *selector,
    position_soa_t *position,
    int pos_idx,
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
        .character = selector->character,
        .input_type = selector->player_input,
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
load_ground_assets_fn(ground_t *ground, int pos_idx)
{
    *ground = (ground_t){ .pos_idx = pos_idx, };
}



void
load_game_assets_fn(
    arena_allocator_t *allocator, 
    game_object_t *game_obj, 
    position_soa_t *position,
    const selector_t *selector,
    const int screen_width, 
    const int screen_height)
{
    player_t player_1 = {0};
    player_t player_2 = {0};
    ground_t ground = {0};

    int ret = position_soa_append_item_fn(
        allocator, position, 
        (position_t){ 
            .x_pos = 0, .y_pos = screen_height - 20, .z_pos = 0, 
            .x_vel = 0, .y_vel = 0, 
            .width = screen_width, .height = screen_height - 20,
        });
    load_ground_assets_fn(&ground, (position->len - 1));
    
    // Player 1 and 2
    player_selector_t play_select[] = {selector->player_1, selector->player_2};
    slice_t frames_slice[] = {(slice_t){0}, (slice_t){0}};
    Texture2D texture[] = {(Texture2D){0}, (Texture2D){0}};
    Rectangle rect[] = {(Rectangle){0}, (Rectangle){0}};
    for (int i = 0; i < 2; i++){
        switch (play_select[i].character) {
            case RYU:
                frames_slice[i] = make_slice(ryu_frames, sizeof(ryu_frames));
                texture[i] = LoadTexture("assets/Ryu.gif");
                rect[i].width = 100;
                rect[i].height = 200;
                break;
        }
    }

    ret = position_soa_append_item_fn(
        allocator, position, 
        (position_t){ 
            .x_pos = 50, .y_pos = position->height[ground.pos_idx] - rect[1].height, .z_pos = 0, 
            .x_vel = 4, .y_vel = 0, 
            .width = rect[0].width, .height = rect[0].height,
        });
    load_player_assets_fn(
        allocator, 
        &player_1, 
        &selector->player_1, 
        position, 
        (position->len - 1), &texture[0], &frames_slice[0], FLIP_RIGHT);
    
    ret = position_soa_append_item_fn(
        allocator, position, 
        (position_t){ 
            .x_pos = screen_width - 100, .y_pos = position->height[ground.pos_idx] - rect[1].height, .z_pos = 0, 
            .x_vel = 4, .y_vel = 0, 
            .width = rect[1].width, .height = rect[1].height,
        });
    load_player_assets_fn(
        allocator, 
        &player_2, 
        &selector->player_2, 
        position, (position->len - 1), &texture[1], &frames_slice[1], FLIP_LEFT);

    *game_obj = (game_object_t){ 
        .player_1 = player_1, 
        .player_2 = player_2, 
        .ground = ground};
}
#endif