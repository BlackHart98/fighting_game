#include "raylib.h"

// Frames will be re-written as a frame generator module
#include "../src/frames.h"

#define WSA_IMPLEMENTATION
#include "../lib/why_so_arena.h"
#define ARRAY_LIST_IMPLEMENTATION
#include "../lib/array_list.h"
#include "../src/load_game_assets.h"
#include "../src/renderer.h"

#include <stdio.h>
#include <stdlib.h>

void
compute_frame_orientation_fn(player_t *player);

void
update_fn(
    game_object_t *game_obj, 
    position_soa_t *position_soa, 
    const int screen_width, 
    const int screen_height, 
    float delta);


void
player_action_fn(
    player_t *player, 
    position_soa_t *position_soa, 
    float delta, 
    const int screen_width, 
    const int screen_height);


void
ai_action_fn(
    player_t *player, 
    player_t *opponent, 
    position_soa_t *position_soa, 
    float delta, 
    const int screen_width, 
    const int screen_height);


void
player_tick_fn(
    player_t *player, 
    position_soa_t *position_soa, 
    float delta, 
    const int screen_width, 
    const int screen_height);


 
int
main(void)
{
    const int screen_width = 800;
    const int screen_height = 450;

    arena_allocator_t gpa = arena_allocator_init_page_default(c_allocator, MB(2));
    InitWindow(screen_width, screen_height, "fighting game");
    {
        game_object_t game_obj = (game_object_t){0};
        position_soa_t position_soa = position_soa_init_capacity_fn(&gpa, 4);
        load_game_assets_fn(
            &gpa, &game_obj, &position_soa,
            &(selector_t){
                .environment = {0},
                .player_1 = (player_selector_t){
                    .character = RYU,
                    .player_input = PLAYER,
                },
                .player_2 = (player_selector_t){
                    .character = RYU,
                    .player_input = AI,
                }
            },
            screen_width, 
            screen_height);
        SetTargetFPS(60); 
        while (!WindowShouldClose()) {
            float delta = GetFrameTime();
            // Update:
            update_fn(&game_obj, &position_soa, screen_width, screen_height, delta);
            // Render:
            render_screen_fn(&game_obj, &position_soa);       
        }
        UnloadTexture(game_obj.player_1.sprite.texture);
        UnloadTexture(game_obj.player_2.sprite.texture);
    }
    CloseWindow();

    // garbage collection
    arena_allocator_reset(&gpa);
    arena_allocator_deinit(&gpa);
    return 0;
}


void
player_action_fn(
    player_t *player, 
    position_soa_t *position_soa, 
    float delta, 
    const int screen_width, const 
    int screen_height)
{
    // Player Actions
    if (player->player_state){
        if (IsKeyDown(KEY_RIGHT)) {
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            if (player->flip == FLIP_RIGHT) player->sprite.current_anim_group = MOVE;
            else player->sprite.current_anim_group = BACKOFF;
        }
        if (IsKeyDown(KEY_LEFT)) {
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            if (player->flip == FLIP_RIGHT) player->sprite.current_anim_group = BACKOFF;
            else player->sprite.current_anim_group = MOVE;
        }
        if (IsKeyPressed(KEY_P)) {
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            player->sprite.current_anim_group = JAB;
        }
        if (IsKeyPressed(KEY_H)) {
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            player->sprite.current_anim_group = LEFT_RIGHT_HOOK;
        }
        if (IsKeyDown(KEY_K)) {
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            player->sprite.current_anim_group = LIGHT_KICK;
        }
        if (IsKeyDown(KEY_R)) {
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            player->sprite.current_anim_group = ROUNDHOUSE;
        }
    }
}


void
ai_action_fn(
    player_t *player, 
    player_t *opponent, 
    position_soa_t *position_soa, 
    float delta, 
    const int screen_width, 
    const int screen_height)
{
    if (player->player_state) {
        float distance = position_soa->x_pos[player->pos_idx] - position_soa->x_pos[opponent->pos_idx];
        // move towards player:
        if (-75 > distance) player->sprite.current_anim_group = MOVE;
        if (75 < distance) player->sprite.current_anim_group = MOVE;
    }
}


void
update_fn(
    game_object_t *game_obj, 
    position_soa_t *position_soa, 
    const int screen_width, 
    const int screen_height, 
    float delta)
{
    // Fix issues with orientation
    // Player Actions
    player_action_fn(&(game_obj->player_1), position_soa, delta, screen_width, screen_height);
    // // AI Actions
    // ai_action_fn(&(game_obj->player_2), &(game_obj->player_1), position_soa, delta, screen_width, screen_height); 

    player_tick_fn(&(game_obj->player_1), position_soa, delta, screen_width, screen_height);
    player_tick_fn(&(game_obj->player_2), position_soa, delta, screen_width, screen_height);


    // Flip players
    if (FLIP_RIGHT == game_obj->player_1.flip 
        && (position_soa->x_pos[game_obj->player_1.pos_idx] > position_soa->x_pos[game_obj->player_2.pos_idx] + 50)
    ) {
        char flip = game_obj->player_1.flip;
        game_obj->player_1.flip = game_obj->player_2.flip;
        game_obj->player_2.flip = flip;
    } else if (FLIP_RIGHT == game_obj->player_2.flip 
        && (position_soa->x_pos[game_obj->player_2.pos_idx] > position_soa->x_pos[game_obj->player_1.pos_idx] + 50)
    ) {
        char flip = game_obj->player_1.flip;
        game_obj->player_1.flip = game_obj->player_2.flip;
        game_obj->player_2.flip = flip;
    }
}


void
compute_frame_orientation_fn(player_t *player)
{
    int anim_group = player->sprite.current_anim_group;
    int current_frame = player->sprite.anim[anim_group].current_frame_idx;
    switch (player->flip){
        case FLIP_LEFT: {
            if (0 <= player->sprite.anim[anim_group].frame_rec[current_frame].width) {
                player->sprite.anim[anim_group].frame_rec[current_frame].width *= -1.0f;
            }
            break;
        }
        case FLIP_RIGHT: {
            if (0 > player->sprite.anim[anim_group].frame_rec[current_frame].width) {
                player->sprite.anim[anim_group].frame_rec[current_frame].width *= -1.0f;
            }
            break;
        }
    }
}


void
player_tick_fn(
    player_t *player, 
    position_soa_t *position_soa, 
    float delta, 
    const int screen_width, 
    const int screen_height)
{
    int anim_group = player->sprite.current_anim_group;
    player->sprite.anim[anim_group].elapsed_time += delta;
    float fps = 1 / player->sprite.anim[anim_group].frame_speed;
    
    switch (anim_group) {
        case MOVE: {
            if (player->flip == FLIP_RIGHT){
                position_soa->x_pos[player->pos_idx] += position_soa->x_vel[player->pos_idx];
                if (position_soa->x_pos[player->pos_idx] >= screen_width - 50) 
                    position_soa->x_pos[player->pos_idx] = screen_width - 50;
            } else {
                position_soa->x_pos[player->pos_idx] -= position_soa->x_vel[player->pos_idx];
                if (position_soa->x_pos[player->pos_idx] <= 50) 
                    position_soa->x_pos[player->pos_idx] = 50;
            }
            break;
        } 
        case BACKOFF: {
            if (player->flip == FLIP_RIGHT){
                position_soa->x_pos[player->pos_idx] -= position_soa->x_vel[player->pos_idx];
                if (position_soa->x_pos[player->pos_idx] <= 50) 
                    position_soa->x_pos[player->pos_idx] = 50;
            } else {
                position_soa->x_pos[player->pos_idx] += position_soa->x_vel[player->pos_idx];
                if (position_soa->x_pos[player->pos_idx] >= screen_width - 50) 
                    position_soa->x_pos[player->pos_idx] = screen_width - 50;
            }
            break;
        }
    }
    if (player->sprite.anim[anim_group].elapsed_time >= fps) {
        player->sprite.anim[anim_group].elapsed_time = 0.0f;
        player->sprite.anim[anim_group].current_frame_idx++;
        if (player->sprite.anim[anim_group].current_frame_idx >= player->sprite.anim[anim_group].frame_count) {
            player->sprite.anim[anim_group].current_frame_idx = 0;
            player->sprite.prev_anim_group = player->sprite.current_anim_group;
            player->sprite.current_anim_group = desc[anim_group].next_anim;
            player->player_state = 1;
        }
    }
    compute_frame_orientation_fn(player);
}