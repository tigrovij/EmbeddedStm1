#include "software_pwm.h"
#include "main.h"
#include <stdbool.h>

#define PWM_PERIOD_MS 10u
#define PWM_INITIAL_ON_MS 5u
#define BUTTON_DEBOUNCE_MS 20u

typedef struct
{
    GPIO_PinState raw_level;
    GPIO_PinState stable_level;
    uint32_t changed_at;
} PwmButton;

static PwmButton up_button;
static PwmButton down_button;
static uint32_t requested_on_ms;
static uint32_t on_ms;
static uint32_t frame_start;
static uint32_t last_tick;

static bool ButtonPressed(PwmButton *button, GPIO_PinState level, uint32_t now)
{
    if (level != button->raw_level)
    {
        button->raw_level = level;
        button->changed_at = now;
    }

    if (level != button->stable_level
        && now - button->changed_at >= BUTTON_DEBOUNCE_MS)
    {
        button->stable_level = level;
        return level == GPIO_PIN_RESET;
    }

    return false;
}

void SoftwarePwm_Init(void)
{
    up_button = (PwmButton){
        .raw_level = GPIO_PIN_SET,
        .stable_level = GPIO_PIN_SET,
    };
    down_button = up_button;
    requested_on_ms = PWM_INITIAL_ON_MS;
    on_ms = requested_on_ms;
    frame_start = HAL_GetTick();
    last_tick = frame_start - 1u;
}

void SoftwarePwm_Update(void)
{
    uint32_t now = HAL_GetTick();
    if (now == last_tick)
    {
        return;
    }
    last_tick = now;

    bool up_pressed = ButtonPressed(
        &up_button,
        HAL_GPIO_ReadPin(PWM_UP_GPIO_Port, PWM_UP_Pin),
        now);
    bool down_pressed = ButtonPressed(
        &down_button,
        HAL_GPIO_ReadPin(PWM_DOWN_GPIO_Port, PWM_DOWN_Pin),
        now);
    if (up_pressed && !down_pressed && requested_on_ms < PWM_PERIOD_MS)
    {
        ++requested_on_ms;
    }
    else if (down_pressed && !up_pressed && requested_on_ms > 0u)
    {
        --requested_on_ms;
    }

    uint32_t phase = now - frame_start;
    if (phase >= PWM_PERIOD_MS)
    {
        phase %= PWM_PERIOD_MS;
        frame_start = now - phase;
        on_ms = requested_on_ms;
    }
    HAL_GPIO_WritePin(
        LED_PWM_GPIO_Port,
        LED_PWM_Pin,
        phase < on_ms ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
