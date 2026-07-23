/**
 *******************************************************************************
 * @file        tiremo_systick.c
 * @brief       Tiremo system tick timing utilities
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "tiremo_systick.h"

extern uint32_t SystemCoreClock;

/** Decremented by SysTick_Handler; used by TIREMO_SysTick_DelayMs(). */
static volatile uint32_t s_tickCounter = 0U;

/** Free-running ms counter since Init. */
static volatile uint32_t s_msTick = 0U;

void SysTick_Handler(void)
{
    if (s_tickCounter > 0U)
    {
        s_tickCounter--;
    }

    s_msTick++;
    TIREMO_SysTick_OnTick1ms();
}

__attribute__((weak)) void TIREMO_SysTick_OnTick1ms(void)
{
    /* Override in Tiremo_Process when MQTT / extra 1 ms work is needed. */
}

void TIREMO_SysTick_Init(void)
{
    s_msTick = 0U;
    SysTick_Config(SystemCoreClock / TIREMO_SYSTICK_1MS_DIV);
}

void TIREMO_SysTick_DelayMs(uint32_t delayMs)
{
    s_tickCounter = delayMs;
    while (s_tickCounter > 0U)
    {
        /* Busy-wait; counter is decremented in SysTick_Handler. */
    }
}

uint32_t TIREMO_SysTick_GetMs(void)
{
    return s_msTick;
}

void SYSTICK_Wait(uint32_t un32TimeMS)
{
    TIREMO_SysTick_DelayMs(un32TimeMS);
}
