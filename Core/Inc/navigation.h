#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <stdint.h>

typedef enum
{
    NAV_SCENE_1_CLEAR_FORWARD = 1,
    NAV_SCENE_2_FRONT_ONLY = 2,
    NAV_SCENE_3_FRONT_LEFT = 3,
    NAV_SCENE_4_FRONT_RIGHT = 4,
    NAV_SCENE_5_FRONT_LEFT_RIGHT = 5
} NavSceneId;

typedef enum
{
    NAV_MOTION_STOP = 0,
    NAV_MOTION_FORWARD = 1,
    NAV_MOTION_BACKWARD = 2,
    NAV_MOTION_TURN_LEFT = 3,
    NAV_MOTION_TURN_RIGHT = 4
} NavMotion;

void Navigation_Init(void);
void Navigation_Process(void);

uint8_t Navigation_GetCounter(void);
NavSceneId Navigation_GetCurrentScene(void);
NavMotion Navigation_GetMotion(void);

#endif /* NAVIGATION_H */
