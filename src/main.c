#include "raylib.h"
#include "../src/frames.h"

#include <stdlib.h>

// #define ABS(x) (0 > x)? -1 * x : x;

enum ANIMATION_GROUP {
    IDLE=0, 
    START, 
    MOVE, 
    BACKOFF, 
    LEFT_RIGHT_HOOK, 
    JAB,
    LIGHT_KICK,
    ROUNDHOUSE,
};


#define START_POSITION_PLAYER_1 (Vector2){50, 50}
#define START_POSITION_PLAYER_2 (Vector2){200, 50}
#define START_VELOCITY (Vector2){4.0f, 0.0f}



typedef struct animation_t {
    float       frame_speed;
    float       elapsed_time;
    int         current_frame;

    // slice_t can hold this
    int         frame_count;
    Rectangle   *frame_rec; // []Rectangle
} animation_t;


typedef struct sprite_t{
    Texture2D texture;
    int current_anim_group;
    int prev_anim_group;

    // Animations
    animation_t idle_anim;
    animation_t start_anim;
    animation_t move_anim;
    animation_t backoff_anim;
    animation_t punch_anim;
    animation_t light_kick_anim;
    animation_t roundhouse_anim;
} sprite_t;


typedef struct player_t {
    sprite_t sprite;
    int pos_idx;
    int vel_idx;
    float health_val;
    char player_state; // ACTIVE | INACTIVE
} player_t;


typedef struct versus_t {
    player_t *player;
} versus_t;


void
update_fn(versus_t *versus, Vector2 *pos, Vector2 *vel, const int screen_width, const int screen_height, float delta);

void
draw_fn(versus_t *versus, Vector2 *pos, Vector2 *vel);

void
player_action_fn(player_t *player, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);

void
ai_action_fn(player_t *player, player_t *opponent, Vector2 *pos, Vector2 *vel, float delta, const int screen_width, const int screen_height);

 
int
main(void)
{
    const int screen_width = 800;
    const int screen_height = 450;
    player_t player_1 = {0};
    player_t player_2 = {0};
    Vector2 pos[] = {START_POSITION_PLAYER_1, START_POSITION_PLAYER_2};
    Vector2 vel[] = {START_VELOCITY, START_VELOCITY};
    InitWindow(screen_width, screen_height, "ryu animation");
    {
        Texture2D ryu_texture = LoadTexture("assets/Ryu.gif");
        sprite_t ryu_sprite = (sprite_t){
            .start_anim = (animation_t){
                .current_frame = 0,
                .frame_rec = ryu_start_frame,
                .frame_speed = 8.0f,
                .frame_count = 9,
                .elapsed_time = 0.0f,
            },
            .idle_anim = (animation_t){
                .current_frame = 0,
                .frame_rec = ryu_idle_frame,
                .frame_speed = 10.0f,
                .frame_count = 6,
                .elapsed_time = 0.0f,
            },
            .move_anim = (animation_t){
                .current_frame = 0,
                .frame_rec = ryu_move_frame,
                .frame_speed = 15.0f,
                .frame_count = 6,
                .elapsed_time = 0.0f,
            },
            .backoff_anim = (animation_t){
                .current_frame = 0,
                .frame_rec = ryu_backoff_frame,
                .frame_speed = 15.0f,
                .frame_count = 6,
                .elapsed_time = 0.0f,
            },
            .punch_anim = (animation_t){
                .current_frame = 0,
                .frame_rec = ryu_punch_frame,
                .frame_speed = 15.0f,
                .frame_count = 9,
                .elapsed_time = 0.0f,
            },
            .light_kick_anim = (animation_t){
                .current_frame = 0,
                .frame_rec = ryu_kick_frame,
                .frame_speed = 15.0f,
                .frame_count = 6,
                .elapsed_time = 0.0f,
            },
            .roundhouse_anim = (animation_t){
                .current_frame = 0,
                .frame_rec = ryu_roundhouse_frame,
                .frame_speed = 15.0f,
                .frame_count = 5,
                .elapsed_time = 0.0f,
            },

            .texture = ryu_texture,
            .current_anim_group = START,
            .prev_anim_group = START,
        };

        // Player 1:
        player_1 = (player_t) { .sprite = ryu_sprite, .pos_idx = 0, .vel_idx = 0, .player_state = 0, };
        // Player 2:
        player_2 = (player_t) { .sprite = ryu_sprite, .pos_idx = 1, .vel_idx = 1, .player_state = 0, };
        versus_t vs = (versus_t){ .player = &(player_t[]){ player_1, player_2 }};
        SetTargetFPS(60); 
        while (!WindowShouldClose()) {
            float delta = GetFrameTime();
            // Update
            update_fn(&vs, pos, vel, screen_width, screen_height, delta);
            // Render
            draw_fn(&vs, pos, vel);       
        }
        UnloadTexture(ryu_texture);
    }
    CloseWindow();
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
        switch (versus->player[i].sprite.current_anim_group){
            case IDLE: {
                versus->player[i].sprite.idle_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.idle_anim.frame_speed;
                if (versus->player[i].sprite.idle_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.idle_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.idle_anim.current_frame++;
                    if (versus->player[i].sprite.idle_anim.current_frame >= versus->player[i].sprite.idle_anim.frame_count) versus->player[i].sprite.idle_anim.current_frame = 0;
                }
                break;
            }
            case START: {
                versus->player[i].sprite.start_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.start_anim.frame_speed;
                if (versus->player[i].sprite.start_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.start_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.start_anim.current_frame++;
                    if (versus->player[i].sprite.start_anim.current_frame >= versus->player[i].sprite.start_anim.frame_count) {
                        versus->player[i].sprite.current_anim_group = IDLE;
                        versus->player[i].player_state = 1;
                    }
                }
                break;
            }
            case MOVE: {
                versus->player[i].sprite.move_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.move_anim.frame_speed;
                pos[versus->player[i].pos_idx].x += vel[versus->player[i].vel_idx].x;
                        if (pos[versus->player[i].pos_idx].x >= screen_width - 50) pos[versus->player[i].pos_idx].x = screen_width - 50;
                if (versus->player[i].sprite.move_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.move_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.move_anim.current_frame++;
                    if (versus->player[i].sprite.move_anim.current_frame >= versus->player[i].sprite.move_anim.frame_count) {
                        versus->player[i].sprite.move_anim.current_frame = 0;
                        versus->player[i].sprite.current_anim_group = IDLE;
                    }
                }
                break;
            }
            case BACKOFF: {
                versus->player[i].sprite.backoff_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.backoff_anim.frame_speed;
                pos[versus->player[i].pos_idx].x -= vel[versus->player[i].vel_idx].x;
                if (pos[versus->player[i].pos_idx].x <= 50) pos[versus->player[i].pos_idx].x = 50;
                if (versus->player[i].sprite.backoff_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.backoff_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.backoff_anim.current_frame++;
                    if (versus->player[i].sprite.backoff_anim.current_frame >= versus->player[i].sprite.backoff_anim.frame_count) {
                        versus->player[i].sprite.backoff_anim.current_frame = 0;
                        versus->player[i].sprite.prev_anim_group = versus->player[i].sprite.current_anim_group;
                        versus->player[i].sprite.current_anim_group = IDLE;
                    }
                }
                break;
            }
            case JAB: {
                versus->player[i].sprite.punch_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.punch_anim.frame_speed;
                if (versus->player[i].sprite.punch_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.punch_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.punch_anim.current_frame++;
                    if (versus->player[i].sprite.punch_anim.current_frame >= 3) {
                        versus->player[i].sprite.punch_anim.current_frame = 3;
                        versus->player[i].sprite.prev_anim_group = versus->player[i].sprite.current_anim_group;
                        versus->player[i].sprite.current_anim_group = IDLE;
                    }
                }
                break;
            }
            case LEFT_RIGHT_HOOK: {
                versus->player[i].sprite.punch_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.punch_anim.frame_speed;
                if (versus->player[i].sprite.punch_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.punch_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.punch_anim.current_frame++;
                    if (versus->player[i].sprite.punch_anim.current_frame >= versus->player[i].sprite.punch_anim.frame_count) {
                        versus->player[i].sprite.punch_anim.current_frame = 0;
                        versus->player[i].sprite.prev_anim_group = versus->player[i].sprite.current_anim_group;
                        versus->player[i].sprite.current_anim_group = IDLE;
                    }
                }
                break;
            }
            case LIGHT_KICK: {
                versus->player[i].sprite.light_kick_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.light_kick_anim.frame_speed;
                if (versus->player[i].sprite.light_kick_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.light_kick_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.light_kick_anim.current_frame++;
                    if (versus->player[i].sprite.light_kick_anim.current_frame >= versus->player[i].sprite.light_kick_anim.frame_count) {
                        versus->player[i].sprite.light_kick_anim.current_frame = 0;
                        versus->player[i].sprite.prev_anim_group = versus->player[i].sprite.current_anim_group;
                        versus->player[i].sprite.current_anim_group = IDLE;
                    }
                }
                break;
            }
            case ROUNDHOUSE: {
                versus->player[i].sprite.roundhouse_anim.elapsed_time += delta;
                float fps = 1 / versus->player[i].sprite.roundhouse_anim.frame_speed;
                if (versus->player[i].sprite.roundhouse_anim.elapsed_time >= fps) {
                    versus->player[i].sprite.roundhouse_anim.elapsed_time = 0.0f;
                    versus->player[i].sprite.roundhouse_anim.current_frame++;
                    if (versus->player[i].sprite.roundhouse_anim.current_frame >= versus->player[i].sprite.roundhouse_anim.frame_count) {
                        versus->player[i].sprite.roundhouse_anim.current_frame = 0;
                        versus->player[i].sprite.prev_anim_group = versus->player[i].sprite.current_anim_group;
                        versus->player[i].sprite.current_anim_group = IDLE;
                    }
                }
                break;
            }
        }
    }
}


void
draw_fn(versus_t *versus, Vector2 *pos, Vector2 *vel)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        for (int i = 0; i < 2; i++) {
            switch (versus->player[i].sprite.current_anim_group){
                case IDLE: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.idle_anim.frame_rec[versus->player[i].sprite.idle_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
                case START: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.start_anim.frame_rec[versus->player[i].sprite.start_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
                case MOVE: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.move_anim.frame_rec[versus->player[i].sprite.move_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
                case BACKOFF: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.backoff_anim.frame_rec[versus->player[i].sprite.backoff_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
                case JAB: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.punch_anim.frame_rec[versus->player[i].sprite.punch_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
                case LEFT_RIGHT_HOOK: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.punch_anim.frame_rec[versus->player[i].sprite.punch_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
                case LIGHT_KICK: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.light_kick_anim.frame_rec[versus->player[i].sprite.light_kick_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
                case ROUNDHOUSE: {
                    DrawTextureRec(
                        versus->player[i].sprite.texture, 
                        versus->player[i].sprite.roundhouse_anim.frame_rec[versus->player[i].sprite.roundhouse_anim.current_frame], 
                        pos[versus->player[i].pos_idx], WHITE);
                    break;
                }
            }
        }
    }
    EndDrawing(); 
}