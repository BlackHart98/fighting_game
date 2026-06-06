#include "raylib.h"

// Frames will be re-written as a frame generator module
#include "../src/frames.h"

#define WSA_IMPLEMENTATION
#include "../lib/why_so_arena.h"
#define ARRAY_LIST_IMPLEMENTATION
#include "../lib/array_list.h"
#include "../src/load_game_assets.h"

#include <stdio.h>
#include <stdlib.h>

void
compute_frame_orientation_fn(player_t *player);

void
update_fn(game_object_t *game_obj, Vector2 *pos, Vector2 *vel, const int screen_width, const int screen_height, float delta);

void
draw_fn(game_object_t *game_obj, Vector2 *pos, Vector2 *vel);

void
player_action_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);

void
ai_action_fn(player_t *player, player_t *opponent, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);


void
draw_character_fn(player_t *player, Vector2 *pos, int current_anim_group);


void
player_tick_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);


void 
draw_environment_fn(game_object_t *game_obj, Vector2 *pos);

 
int
main(void)
{
    const int screen_width = 800;
    const int screen_height = 450;
    player_t player_1 = {0};
    player_t player_2 = {0};
    ground_t ground = {0};
    Vector2 pos[] = {START_POSITION_PLAYER_1, START_POSITION_PLAYER_2, START_POSITION_PLAYER_3};
    Vector2 vel[] = {START_VELOCITY, START_VELOCITY};

    arena_allocator_t gpa = arena_allocator_init_page_default(c_allocator, MB(2));
    InitWindow(screen_width, screen_height, "fighting game");
    {
        Texture2D texture = LoadTexture("assets/Ryu.gif");

        slice_t frames_slice = make_slice(ryu_frames, sizeof(ryu_frames));
        load_player_assets_fn(&gpa, &player_1, 0, 0, &texture, &frames_slice, FLIP_RIGHT);
        load_player_assets_fn(&gpa, &player_2, 1, 1, &texture, &frames_slice, FLIP_LEFT);
        load_ground_assets_fn(&ground, 2, screen_width, screen_height);
        game_object_t vs = (game_object_t){ .player = (player_t[]){ player_1, player_2 }, .ground = ground};
        SetTargetFPS(60); 
        while (!WindowShouldClose()) {
            float delta = GetFrameTime();
            // Update:
            update_fn(&vs, pos, vel, screen_width, screen_height, delta);
            // Render:
            draw_fn(&vs, pos, vel);       
        }
        UnloadTexture(texture);
    }
    CloseWindow();

    // garbage collection
    arena_allocator_reset(&gpa);
    arena_allocator_deinit(&gpa);
    return 0;
}


void
player_action_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height)
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
ai_action_fn(player_t *player, player_t *opponent, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height)
{
    if (player->player_state) {
        float distance = pos[0].x - pos[1].x;
        // move towards player:
        if (-75 > distance) player->sprite.current_anim_group = MOVE;
        if (75 < distance) player->sprite.current_anim_group = MOVE;
    }
}


void
update_fn(game_object_t *game_obj, Vector2 *pos, Vector2 *vel, const int screen_width, const int screen_height, float delta)
{
    // Fix issues with orientation
    // Player Actions
    player_action_fn(&(game_obj->player[0]), pos, vel, delta, screen_width, screen_height);
    // AI Actions
    ai_action_fn(&(game_obj->player[1]), &(game_obj->player[0]), pos, vel, delta, screen_width, screen_height); 

    for (int i = 0; i < 2; i++) {
        player_tick_fn(&(game_obj->player[i]), pos, vel, delta, screen_width, screen_height);
    }

    // Flip players
    if (FLIP_RIGHT == game_obj->player[0].flip && (pos[game_obj->player[0].pos_idx].x > pos[game_obj->player[1].pos_idx].x + 50)) {
        char flip = game_obj->player[0].flip;
        game_obj->player[0].flip = game_obj->player[1].flip;
        game_obj->player[1].flip = flip;
    } else if (FLIP_RIGHT == game_obj->player[1].flip && (pos[game_obj->player[1].pos_idx].x > pos[game_obj->player[0].pos_idx].x + 50)) {
        char flip = game_obj->player[0].flip;
        game_obj->player[0].flip = game_obj->player[1].flip;
        game_obj->player[1].flip = flip;
    }
}


void
draw_fn(game_object_t *game_obj, Vector2 *pos, Vector2 *vel)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        draw_environment_fn(game_obj, pos);
        for (int i = 0; i < 2; i++) {
            draw_character_fn(&(game_obj->player[i]), pos, game_obj->player[i].sprite.current_anim_group);
        }
    }
    EndDrawing(); 
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
draw_character_fn(player_t *player, Vector2 *pos, int current_anim_group)
{
    int anim_group = player->sprite.current_anim_group;
    DrawTextureRec(
        player->sprite.texture, 
        player->sprite.anim[anim_group].frame_rec[player->sprite.anim[anim_group].current_frame_idx], 
        pos[player->pos_idx], WHITE);
}


void 
draw_environment_fn(game_object_t *game_obj, Vector2 *pos)
{
    DrawRectangle(
        pos[game_obj->ground.pos_idx].x, 
        pos[game_obj->ground.pos_idx].y, 
        game_obj->ground.width, 
        game_obj->ground.height, 
        BLACK);
}


void
player_tick_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height)
{
    int anim_group = player->sprite.current_anim_group;
    player->sprite.anim[anim_group].elapsed_time += delta;
    float fps = 1 / player->sprite.anim[anim_group].frame_speed;
    
    switch (anim_group) {
        case MOVE: {
            if (player->flip == FLIP_RIGHT){
                pos[player->pos_idx].x += vel[player->vel_idx].x;
                if (pos[player->pos_idx].x >= screen_width - 50) pos[player->pos_idx].x = screen_width - 50;
            } else {
                pos[player->pos_idx].x -= vel[player->vel_idx].x;
                if (pos[player->pos_idx].x <= 50) pos[player->pos_idx].x = 50;
            }
            break;
        } 
        case BACKOFF: {
            if (player->flip == FLIP_RIGHT){
                pos[player->pos_idx].x -= vel[player->vel_idx].x;
                if (pos[player->pos_idx].x <= 50) pos[player->pos_idx].x = 50;
            } else {
                pos[player->pos_idx].x += vel[player->vel_idx].x;
                if (pos[player->pos_idx].x >= screen_width - 50) pos[player->pos_idx].x = screen_width - 50;
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