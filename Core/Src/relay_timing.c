#include "relay_timing.h"
#include "main.h"
#include <stdio.h>

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;

#define RELAY_ON_LEVEL GPIO_PIN_SET
#define RELAY_OFF_LEVEL GPIO_PIN_RESET
#define RELAY_CYCLES 10u
#define RELAY_QUIET_US 10000u
#define RELAY_TIMEOUT_MS 250u

static volatile uint32_t last_edge_us;

void RelayTiming_OnContactEdge(void)
{
    last_edge_us = __HAL_TIM_GET_COUNTER(&htim2);
}

static uint8_t
MeasureRelay(GPIO_PinState drive, GPIO_PinState contact, uint32_t *settled_us)
{
    if (HAL_GPIO_ReadPin(RELAY_SENSE_GPIO_Port, RELAY_SENSE_Pin) == contact)
    {
        return 0u;
    }

    uint32_t start_ms = HAL_GetTick();
    uint32_t start_us = __HAL_TIM_GET_COUNTER(&htim2);
    last_edge_us = start_us; /* No edge recorded for this transition yet. */
    HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port, RELAY_CTRL_Pin, drive);

    while (HAL_GetTick() - start_ms < RELAY_TIMEOUT_MS)
    {
        uint32_t edge_us = last_edge_us;
        uint32_t now_us = __HAL_TIM_GET_COUNTER(&htim2);

        if (edge_us != start_us && now_us - edge_us >= RELAY_QUIET_US
            && HAL_GPIO_ReadPin(RELAY_SENSE_GPIO_Port, RELAY_SENSE_Pin)
                   == contact
            && last_edge_us == edge_us)
        {
            *settled_us = edge_us - start_us;
            return 1u;
        }
    }

    return 0u;
}

static void SendLine(const char *line, int length)
{
    if (length > 0)
    {
        HAL_UART_Transmit(
            &huart2,
            (const uint8_t *)line,
            (uint16_t)length,
            100u);
    }
}

void RelayTiming_Run(void)
{
    HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port, RELAY_CTRL_Pin, RELAY_OFF_LEVEL);
    HAL_TIM_Base_Start(&htim2);
    HAL_Delay(200u);

    char line[128];
    uint32_t on_settled_sum = 0, off_settled_sum = 0;

    for (unsigned cycle = 1; cycle <= RELAY_CYCLES; ++cycle)
    {
        uint32_t on_settled, off_settled;
        if (!MeasureRelay(RELAY_ON_LEVEL, GPIO_PIN_RESET, &on_settled))
        {
            HAL_GPIO_WritePin(
                RELAY_CTRL_GPIO_Port,
                RELAY_CTRL_Pin,
                RELAY_OFF_LEVEL);
            SendLine(
                line,
                snprintf(line, sizeof line, "Cycle %u: ON failed\r\n", cycle));
            return;
        }
        HAL_Delay(100u);
        if (!MeasureRelay(RELAY_OFF_LEVEL, GPIO_PIN_SET, &off_settled))
        {
            HAL_GPIO_WritePin(
                RELAY_CTRL_GPIO_Port,
                RELAY_CTRL_Pin,
                RELAY_OFF_LEVEL);
            SendLine(
                line,
                snprintf(line, sizeof line, "Cycle %u: OFF failed\r\n", cycle));
            return;
        }
        on_settled_sum += on_settled;
        off_settled_sum += off_settled;
        SendLine(
            line,
            snprintf(
                line,
                sizeof line,
                "%u: ON %lu us, OFF %lu us\r\n",
                cycle,
                (unsigned long)on_settled,
                (unsigned long)off_settled));
        HAL_Delay(100u);
    }

    SendLine(
        line,
        snprintf(
            line,
            sizeof line,
            "Average: ON %lu us, OFF %lu us\r\n",
            (unsigned long)(on_settled_sum / RELAY_CYCLES),
            (unsigned long)(off_settled_sum / RELAY_CYCLES)));
}
