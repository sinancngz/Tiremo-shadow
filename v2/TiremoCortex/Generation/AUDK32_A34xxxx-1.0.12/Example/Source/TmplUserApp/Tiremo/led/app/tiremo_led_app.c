/**
 *******************************************************************************
 * @file        tiremo_led_app.c
 * @brief       Tiremo LED application layer implementation
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_led_app.h"

#include "../../common/tiremo_systick.h"

void TIREMO_LED_App_Blink(TiremoLedId_e ledId, uint32_t onMs, uint32_t offMs)
{
    TIREMO_LED_On(ledId);
    TIREMO_SysTick_DelayMs(onMs);

    TIREMO_LED_Off(ledId);
    TIREMO_SysTick_DelayMs(offMs);
}

void TIREMO_LED_App_BlinkAll(uint32_t onMs, uint32_t offMs)
{
    TIREMO_LED_AllOn();
    TIREMO_SysTick_DelayMs(onMs);

    TIREMO_LED_AllOff();
    TIREMO_SysTick_DelayMs(offMs);
}

void TIREMO_LED_App_Toggle(TiremoLedId_e ledId)
{
    TIREMO_LED_Toggle(ledId);
}

void TIREMO_LED_App_Chase(uint32_t stepMs)
{
    uint32_t index;

    TIREMO_LED_AllOff();

    for (index = 0U; index < TIREMO_LED_COUNT; index++)
    {
        TIREMO_LED_On((TiremoLedId_e)index);
        TIREMO_SysTick_DelayMs(stepMs);
        TIREMO_LED_Off((TiremoLedId_e)index);
    }
}

void TIREMO_LED_App_Wave(uint32_t stepMs)
{
    uint32_t index;

    /* Rising edge: turn on LEDs one by one, previous LEDs stay on. */
    for (index = 0U; index < TIREMO_LED_COUNT; index++)
    {
        TIREMO_LED_On((TiremoLedId_e)index);
        TIREMO_SysTick_DelayMs(stepMs);
    }

    /* Falling edge: turn off LEDs one by one, previous LEDs stay off. */
    for (index = 0U; index < TIREMO_LED_COUNT; index++)
    {
        TIREMO_LED_Off((TiremoLedId_e)index);
        TIREMO_SysTick_DelayMs(stepMs);
    }
}
