#ifndef FRAMES_H
#define FRAMES_H
#include "raylib.h"
#include <stdlib.h>

enum ANIMATION_GROUP {
    START=0,
    IDLE,  
    MOVE, 
    BACKOFF, 
    LEFT_RIGHT_HOOK, 
    JAB,
    LIGHT_KICK,
    ROUNDHOUSE,
};

typedef struct frame_rec_slice_t {
    Rectangle   *frame_rec;
    size_t      len;
    float       frame_speed;
} frame_rec_slice_t;

// Fight frames
// Ryu
frame_rec_slice_t ryu_frames[] = (frame_rec_slice_t[]){
    [START]          = (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            (Rectangle){ .x = 12, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 87, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 165, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 240, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 315, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 393, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 462, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 532, .y = 5, .height = 102, .width = 70 },
            (Rectangle){ .x = 27, .y = 120, .height = 102, .width = 70 },
        },
        .frame_speed = 8.0f,
        .len = 9 
    },
    [IDLE]           = (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            (Rectangle){ .x = 110, .y = 120, .height = 102, .width = 70 },
            (Rectangle){ .x = 188, .y = 120, .height = 102, .width = 70 },
            (Rectangle){ .x = 270, .y = 120, .height = 102, .width = 70 },
            (Rectangle){ .x = 348, .y = 120, .height = 102, .width = 70 },
            (Rectangle){ .x = 425, .y = 120, .height = 102, .width = 70 },
            (Rectangle){ .x = 502, .y = 120, .height = 102, .width = 70 },
        },
        .frame_speed = 10.0f,
        .len = 6 
    },
    [MOVE]           = (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            (Rectangle){ .x = 58, .y = 238, .height = 102, .width = 70 },
            (Rectangle){ .x = 139, .y = 238, .height = 102, .width = 70 },
            (Rectangle){ .x = 218, .y = 238, .height = 102, .width = 70 },
            (Rectangle){ .x = 295, .y = 238, .height = 102, .width = 70 },
            (Rectangle){ .x = 369, .y = 238, .height = 102, .width = 70 },
            (Rectangle){ .x = 446, .y = 238, .height = 102, .width = 70 },
        },
        .frame_speed = 15.0f,
        .len = 6 
    },
    [BACKOFF]        = (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            (Rectangle){ .x = 60, .y = 345, .height = 102, .width = 70 },
            (Rectangle){ .x = 139, .y = 345, .height = 102, .width = 70 },
            (Rectangle){ .x = 218, .y = 345, .height = 102, .width = 70 },
            (Rectangle){ .x = 295, .y = 345, .height = 102, .width = 70 },
            (Rectangle){ .x = 369, .y = 345, .height = 102, .width = 70 },
            (Rectangle){ .x = 446, .y = 345, .height = 102, .width = 70 },
        },
        .frame_speed = 15.0f,
        .len = 6 
    },
    [LEFT_RIGHT_HOOK]= (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            // Left-Right hook
            (Rectangle){ .x = 103, .y = 818, .height = 102, .width = 70 },
            (Rectangle){ .x = 190, .y = 818, .height = 102, .width = 109 },
            (Rectangle){ .x = 308, .y = 818, .height = 102, .width = 70 },
            (Rectangle){ .x = 404, .y = 818, .height = 102, .width = 109 },
            (Rectangle){ .x = 518, .y = 818, .height = 102, .width = 70 },
            (Rectangle){ .x = 4, .y = 936, .height = 102, .width = 70 },
        },
        .frame_speed = 15.0f,
        .len = 6 
    },
    [JAB]            = (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            // Jab
            (Rectangle){ .x = 430, .y = 696, .height = 102, .width = 70 },
            (Rectangle){ .x = 522, .y = 696, .height = 102, .width = 110 },
            (Rectangle){ .x = 22, .y = 818, .height = 102, .width = 70 },
        },
        .frame_speed = 15.0f,
        .len = 3 
    },
    [LIGHT_KICK]     = (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            // Light kick
            (Rectangle){ .x = 172, .y = 930, .height = 102, .width = 99 },
            (Rectangle){ .x = 261, .y = 930, .height = 102, .width = 69 },
            (Rectangle){ .x = 338, .y = 930, .height = 102, .width = 109 },
            (Rectangle){ .x = 455, .y = 930, .height = 102, .width = 70 },
            (Rectangle){ .x = 536, .y = 930, .height = 102, .width = 99 },
            (Rectangle){ .x = 2, .y = 930, .height = 102, .width = 80 },
        },
        .frame_speed = 15.0f,
        .len = 6 
    },
    [ROUNDHOUSE]     = (frame_rec_slice_t){
        .frame_rec = (Rectangle[]){
            // Roundhouse
            (Rectangle){ .x = 398, .y = 1050, .height = 102, .width = 70 },
            (Rectangle){ .x = 480, .y = 1040, .height = 102, .width = 109 },
            (Rectangle){ .x = 7, .y = 1160, .height = 102, .width = 109 },
            (Rectangle){ .x = 128, .y = 1160, .height = 102, .width = 100 },
            (Rectangle){ .x = 223, .y = 1160, .height = 102, .width = 70 },
        },
        .frame_speed = 15.0f,
        .len = 4 
    },
};



#endif