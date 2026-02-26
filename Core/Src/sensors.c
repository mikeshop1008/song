#include "sensors.h"

#include "app_config.h"
#include "pin_map.h"

static ADC_HandleTypeDef *g_adc = NULL;
static SensorSnapshot g_snapshot;

static uint16_t g_opb_filter = 0U;
static uint16_t g_front_filter = 0U;
static uint16_t g_left_filter = 0U;
static uint16_t g_right_filter = 0U;

static uint8_t g_mark_stable = 0U;
static uint8_t g_mark_candidate = 0U;
static uint32_t g_mark_candidate_since = 0U;
static uint32_t g_mark_last_edge_ms = 0U;
static uint8_t g_mark_edge_latched = 0U;

static uint16_t FilterIir(uint16_t previous, uint16_t input)
{
    if (previous == 0U)
    {
        return input;
    }
    return (uint16_t)(((uint32_t)previous * 3U + (uint32_t)input) / 4U);
}

static uint16_t Sensors_ReadChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef config = {0};

    if (g_adc == NULL)
    {
        return 0U;
    }

    config.Channel = channel;
    config.Rank = 1U;
    config.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    config.Offset = 0U;
    if (HAL_ADC_ConfigChannel(g_adc, &config) != HAL_OK)
    {
        return 0U;
    }

    if (HAL_ADC_Start(g_adc) != HAL_OK)
    {
        return 0U;
    }
    if (HAL_ADC_PollForConversion(g_adc, 5U) != HAL_OK)
    {
        (void)HAL_ADC_Stop(g_adc);
        return 0U;
    }

    {
        uint16_t value = (uint16_t)HAL_ADC_GetValue(g_adc);
        (void)HAL_ADC_Stop(g_adc);
        return value;
    }
}

static uint8_t IsMarkRawDetected(uint16_t adc_value)
{
#if OPB704_ACTIVE_LOW
    return (adc_value < MARK_ADC_THRESHOLD) ? 1U : 0U;
#else
    return (adc_value > MARK_ADC_THRESHOLD) ? 1U : 0U;
#endif
}

void Sensors_Init(ADC_HandleTypeDef *hadc)
{
    g_adc = hadc;

    g_snapshot.opb704_adc = 0U;
    g_snapshot.front_adc = 0U;
    g_snapshot.left_adc = 0U;
    g_snapshot.right_adc = 0U;
    g_snapshot.mark_detected = 0U;
    g_snapshot.front_blocked = 0U;
    g_snapshot.left_blocked = 0U;
    g_snapshot.right_blocked = 0U;

    g_mark_stable = 0U;
    g_mark_candidate = 0U;
    g_mark_candidate_since = HAL_GetTick();
    g_mark_last_edge_ms = 0U;
    g_mark_edge_latched = 0U;
}

void Sensors_Update(void)
{
    uint16_t raw_opb;
    uint16_t raw_front;
    uint16_t raw_left;
    uint16_t raw_right;
    uint8_t mark_raw;
    uint32_t now;

    raw_opb = Sensors_ReadChannel(OPB704_ADC_CHANNEL);
    raw_front = Sensors_ReadChannel(OBST_FRONT_ADC_CHANNEL);
    raw_left = Sensors_ReadChannel(OBST_LEFT_ADC_CHANNEL);
    raw_right = Sensors_ReadChannel(OBST_RIGHT_ADC_CHANNEL);

    g_opb_filter = FilterIir(g_opb_filter, raw_opb);
    g_front_filter = FilterIir(g_front_filter, raw_front);
    g_left_filter = FilterIir(g_left_filter, raw_left);
    g_right_filter = FilterIir(g_right_filter, raw_right);

    g_snapshot.opb704_adc = g_opb_filter;
    g_snapshot.front_adc = g_front_filter;
    g_snapshot.left_adc = g_left_filter;
    g_snapshot.right_adc = g_right_filter;

    g_snapshot.front_blocked = (g_snapshot.front_adc >= OBSTACLE_ADC_THRESHOLD_25CM) ? 1U : 0U;
    g_snapshot.left_blocked = (g_snapshot.left_adc >= OBSTACLE_ADC_THRESHOLD_25CM) ? 1U : 0U;
    g_snapshot.right_blocked = (g_snapshot.right_adc >= OBSTACLE_ADC_THRESHOLD_25CM) ? 1U : 0U;

    mark_raw = IsMarkRawDetected(g_snapshot.opb704_adc);
    now = HAL_GetTick();

    if (mark_raw != g_mark_candidate)
    {
        g_mark_candidate = mark_raw;
        g_mark_candidate_since = now;
    }

    if ((now - g_mark_candidate_since >= MARK_DEBOUNCE_MS) && (g_mark_stable != g_mark_candidate))
    {
        g_mark_stable = g_mark_candidate;
        if ((g_mark_stable != 0U) && (now - g_mark_last_edge_ms >= MARK_REARM_MS))
        {
            g_mark_edge_latched = 1U;
            g_mark_last_edge_ms = now;
        }
    }

    g_snapshot.mark_detected = g_mark_stable;
}

const SensorSnapshot *Sensors_GetSnapshot(void)
{
    return &g_snapshot;
}

uint8_t Sensors_ConsumeMarkEdge(void)
{
    uint8_t latched = g_mark_edge_latched;
    g_mark_edge_latched = 0U;
    return latched;
}
