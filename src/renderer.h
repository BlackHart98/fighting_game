#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include <stdlib.h>

#include "../lib/why_so_arena.h"
#include "../lib/array_list.h"
#include "../src/load_game_assets.h"

void
draw_fighter_fn(game_object_t *game_obj, position_soa_t *position_soa);

void
draw_character_fn(player_t *player, position_soa_t *position_soa, int current_anim_group);


void 
draw_environment_fn(game_object_t *game_obj, position_soa_t *position_soa);


void 
render_screen_fn(game_object_t *game_obj, position_soa_t *position_soa);


void
draw_fighter_fn(game_object_t *game_obj, position_soa_t *position_soa)
{
    draw_character_fn(&(game_obj->player_1), position_soa, game_obj->player_1.sprite.current_anim_group);
    draw_character_fn(&(game_obj->player_2), position_soa, game_obj->player_2.sprite.current_anim_group);
}



void
draw_character_fn(player_t *player, position_soa_t *position_soa, int current_anim_group)
{
    int anim_group = player->sprite.current_anim_group;
    Rectangle dest_rec = (Rectangle){
        .height = position_soa->height[player->pos_idx],
        .width = position_soa->width[player->pos_idx],
        .x = position_soa->x_pos[player->pos_idx],
        .y = position_soa->y_pos[player->pos_idx],
    };
    DrawTexturePro(
        player->sprite.texture, 
        player->sprite.anim[anim_group].frame_rec[player->sprite.anim[anim_group].current_frame_idx], 
        dest_rec, (Vector2){0.0f, 0.0f}, 
        0.0f, WHITE);
    DrawRectangleLines(dest_rec.x, dest_rec.y, dest_rec.width, dest_rec.height, RED);
}


void 
draw_environment_fn(game_object_t *game_obj, position_soa_t *position_soa)
{
    position_t pos_vel = position_soa_get_position(position_soa, game_obj->ground.pos_idx);
    DrawRectangle(
        pos_vel.x_pos, 
        pos_vel.y_pos, 
        pos_vel.width, 
        pos_vel.height, 
        BLACK);
}


void 
render_screen_fn(game_object_t *game_obj, position_soa_t *position_soa)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        draw_environment_fn(game_obj, position_soa);
        draw_fighter_fn(game_obj, position_soa);
    }
    EndDrawing(); 
}


#endif