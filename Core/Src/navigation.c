#include "navigation.h"

#include "app_config.h"
#include "buzzer.h"
#include "indicators.h"
#include "motor.h"
#include "sensors.h"
#include "seven_seg.h"

typedef enum
{
    COUNT_MODE_UP = 0,
    COUNT_MODE_DOWN = 1
} CountMode;

typedef enum
{
    ACTION_NONE = 0,
    ACTION_PAUSE,
    ACTION_REVERSE,
    ACTION_BACKOFF,
    ACTION_TURN_LEFT_90,
    ACTION_TURN_RIGHT_90,
    ACTION_U_TURN_180
} ActionType;

typedef struct
{
    ActionType type;
    uint32_t duration_ms;
} TimedAction;

#define ACTION_QUEUE_CAPACITY 6U

static TimedAction g_action_queue[ACTION_QUEUE_CAPACITY];
static uint8_t g_action_count = 0U;

static TimedAction g_active_action;
static uint8_t g_active_action_valid = 0U;
static uint32_t g_active_action_start_ms = 0U;

static uint8_t g_counter = 0U;
static CountMode g_count_mode = COUNT_MODE_UP;
static NavSceneId g_scene = NAV_SCENE_1_CLEAR_FORWARD;
static NavMotion g_motion = NAV_MOTION_STOP;

static uint8_t g_halted = 0U;
static uint8_t g_scene5_countdown_mode = 0U;
static uint8_t g_scene2_turn_toggle = 0U;
static uint8_t g_last_front_blocked = 0U;

static void ActionQueue_Clear(void)
{
    g_action_count = 0U;
}

static uint8_t ActionQueue_Push(ActionType type, uint32_t duration_ms)
{
    if (g_action_count >= ACTION_QUEUE_CAPACITY)
    {
        return 0U;
    }

    g_action_queue[g_action_count].type = type;
    g_action_queue[g_action_count].duration_ms = duration_ms;
    ++g_action_count;
    return 1U;
}

static uint8_t ActionQueue_Pop(TimedAction *action)
{
    uint8_t i;

    if ((action == NULL) || (g_action_count == 0U))
    {
        return 0U;
    }

    *action = g_action_queue[0];

    for (i = 1U; i < g_action_count; ++i)
    {
        g_action_queue[i - 1U] = g_action_queue[i];
    }

    --g_action_count;
    return 1U;
}

static void StopWithCompleteSignal(void)
{
    g_halted = 1U;
    g_active_action_valid = 0U;
    ActionQueue_Clear();
    g_motion = NAV_MOTION_STOP;

    Motor_Stop();
    SevenSeg_ShowNumber(g_counter);
    Buzzer_BeepPattern(2U, BEEP_DONE_ON_MS, BEEP_DONE_OFF_MS);
    Indicators_BlinkBoth(2U, 80U, 80U);
}

static void HandleMarkEvent(void)
{
    if (Sensors_ConsumeMarkEdge() == 0U)
    {
        return;
    }

    Buzzer_BeepBlocking(BEEP_MARK_MS);

    if (g_count_mode == COUNT_MODE_UP)
    {
        if (g_counter < COUNTER_MAX_VALUE)
        {
            ++g_counter;
        }
    }
    else
    {
        if (g_counter > 0U)
        {
            --g_counter;
        }
    }

    SevenSeg_ShowNumber(g_counter);

    if ((g_scene5_countdown_mode != 0U) && (g_counter == 0U))
    {
        StopWithCompleteSignal();
    }
}

static void HandleFrontObstacleEdge(const SensorSnapshot *snapshot)
{
    if ((snapshot->front_blocked != 0U) && (g_last_front_blocked == 0U))
    {
        Buzzer_BeepBlocking(BEEP_OBSTACLE_MS);
    }

    g_last_front_blocked = snapshot->front_blocked;
}

static void ApplyAction(ActionType type)
{
    switch (type)
    {
    case ACTION_PAUSE:
        g_motion = NAV_MOTION_STOP;
        Motor_SetSpeed(0U, 0U);
        Motor_Stop();
        break;

    case ACTION_REVERSE:
    case ACTION_BACKOFF:
        g_motion = NAV_MOTION_BACKWARD;
        Motor_SetSpeed(MOTOR_SPEED_REVERSE_PERCENT, MOTOR_SPEED_REVERSE_PERCENT);
        Motor_Backward();
        g_count_mode = COUNT_MODE_DOWN;
        break;

    case ACTION_TURN_LEFT_90:
        g_motion = NAV_MOTION_TURN_LEFT;
        Motor_SetSpeed(MOTOR_SPEED_TURN_PERCENT, MOTOR_SPEED_TURN_PERCENT);
        Motor_TurnLeftInPlace();
        break;

    case ACTION_TURN_RIGHT_90:
    case ACTION_U_TURN_180:
        g_motion = NAV_MOTION_TURN_RIGHT;
        Motor_SetSpeed(MOTOR_SPEED_TURN_PERCENT, MOTOR_SPEED_TURN_PERCENT);
        Motor_TurnRightInPlace();
        break;

    default:
        g_motion = NAV_MOTION_STOP;
        Motor_SetSpeed(0U, 0U);
        Motor_Stop();
        break;
    }

    if ((type != ACTION_REVERSE) && (type != ACTION_BACKOFF))
    {
        if (g_scene5_countdown_mode != 0U)
        {
            g_count_mode = COUNT_MODE_DOWN;
        }
        else
        {
            g_count_mode = COUNT_MODE_UP;
        }
    }
}

static void StartNextActionIfIdle(void)
{
    if ((g_halted != 0U) || (g_active_action_valid != 0U))
    {
        return;
    }

    if (ActionQueue_Pop(&g_active_action) == 0U)
    {
        return;
    }

    g_active_action_valid = 1U;
    g_active_action_start_ms = HAL_GetTick();
    ApplyAction(g_active_action.type);
}

static void ProcessActiveAction(void)
{
    uint32_t now;

    if (g_active_action_valid == 0U)
    {
        return;
    }

    now = HAL_GetTick();
    if ((now - g_active_action_start_ms) < g_active_action.duration_ms)
    {
        return;
    }

    g_active_action_valid = 0U;
    g_motion = NAV_MOTION_STOP;
    Motor_Stop();
}

static void PlanScene2(void)
{
    ActionQueue_Clear();
    g_scene5_countdown_mode = 0U;

    (void)ActionQueue_Push(ACTION_PAUSE, PAUSE_BEFORE_REVERSE_MS);
    (void)ActionQueue_Push(ACTION_REVERSE, REVERSE_LONG_MS);
    if (g_scene2_turn_toggle == 0U)
    {
        (void)ActionQueue_Push(ACTION_TURN_LEFT_90, TURN_90_MS);
    }
    else
    {
        (void)ActionQueue_Push(ACTION_TURN_RIGHT_90, TURN_90_MS);
    }

    g_scene2_turn_toggle ^= 1U;
}

static void PlanScene3(void)
{
    ActionQueue_Clear();
    g_scene5_countdown_mode = 0U;

    (void)ActionQueue_Push(ACTION_BACKOFF, BACKOFF_SHORT_MS);
    (void)ActionQueue_Push(ACTION_TURN_RIGHT_90, TURN_90_MS);
}

static void PlanScene4(void)
{
    ActionQueue_Clear();
    g_scene5_countdown_mode = 0U;

    (void)ActionQueue_Push(ACTION_BACKOFF, BACKOFF_SHORT_MS);
    (void)ActionQueue_Push(ACTION_TURN_LEFT_90, TURN_90_MS);
}

static void PlanScene5(void)
{
    ActionQueue_Clear();
    g_scene5_countdown_mode = 1U;
    g_count_mode = COUNT_MODE_DOWN;

    (void)ActionQueue_Push(ACTION_BACKOFF, BACKOFF_SHORT_MS);
    (void)ActionQueue_Push(ACTION_U_TURN_180, TURN_180_MS);
}

void Navigation_Init(void)
{
    g_action_count = 0U;
    g_active_action_valid = 0U;
    g_counter = 0U;
    g_count_mode = COUNT_MODE_UP;
    g_scene = NAV_SCENE_1_CLEAR_FORWARD;
    g_motion = NAV_MOTION_STOP;
    g_halted = 0U;
    g_scene5_countdown_mode = 0U;
    g_scene2_turn_toggle = 0U;
    g_last_front_blocked = 0U;

    SevenSeg_ShowNumber(0);
    Motor_Stop();
}

void Navigation_Process(void)
{
    const SensorSnapshot *snapshot;
    uint8_t any_obstacle;

    Sensors_Update();
    snapshot = Sensors_GetSnapshot();

    any_obstacle = (uint8_t)((snapshot->front_blocked != 0U) ||
                             (snapshot->left_blocked != 0U) ||
                             (snapshot->right_blocked != 0U));
    Indicators_SetObstacleLed(any_obstacle);
    Indicators_SetMarkLed(snapshot->mark_detected);

    HandleMarkEvent();
    HandleFrontObstacleEdge(snapshot);

    if (g_halted != 0U)
    {
        g_motion = NAV_MOTION_STOP;
        Motor_Stop();
        return;
    }

    ProcessActiveAction();
    StartNextActionIfIdle();

    if ((g_active_action_valid != 0U) || (g_action_count != 0U))
    {
        return;
    }

    /*
     * Priority rule:
     * If front is clear, always go forward regardless of left/right obstacles.
     */
    if (snapshot->front_blocked == 0U)
    {
        g_scene = NAV_SCENE_1_CLEAR_FORWARD;
        if (g_scene5_countdown_mode != 0U)
        {
            g_count_mode = COUNT_MODE_DOWN;
            if (g_counter == 0U)
            {
                StopWithCompleteSignal();
                return;
            }
        }
        else
        {
            g_count_mode = COUNT_MODE_UP;
        }

        g_motion = NAV_MOTION_FORWARD;
        Motor_SetSpeed(MOTOR_SPEED_FORWARD_PERCENT, MOTOR_SPEED_FORWARD_PERCENT);
        Motor_Forward();
        return;
    }

    if ((snapshot->left_blocked == 0U) && (snapshot->right_blocked == 0U))
    {
        g_scene = NAV_SCENE_2_FRONT_ONLY;
        PlanScene2();
    }
    else if ((snapshot->left_blocked != 0U) && (snapshot->right_blocked == 0U))
    {
        g_scene = NAV_SCENE_3_FRONT_LEFT;
        PlanScene3();
    }
    else if ((snapshot->left_blocked == 0U) && (snapshot->right_blocked != 0U))
    {
        g_scene = NAV_SCENE_4_FRONT_RIGHT;
        PlanScene4();
    }
    else
    {
        g_scene = NAV_SCENE_5_FRONT_LEFT_RIGHT;
        PlanScene5();
    }

    StartNextActionIfIdle();
}

uint8_t Navigation_GetCounter(void)
{
    return g_counter;
}

NavSceneId Navigation_GetCurrentScene(void)
{
    return g_scene;
}

NavMotion Navigation_GetMotion(void)
{
    return g_motion;
}
