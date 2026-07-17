/**
 *******************************************************************************
 * @file        tiremo_systick.h
 * @brief       Tiremo system tick timing utilities
 *
 * @details     Provides millisecond-resolution timing based on the ARM Cortex-M
 *              SysTick timer. Shared by all Tiremo component libraries (LED,
 *              sensors, etc.).
 *
 *              Usage:
 *              @code
 *              TIREMO_SysTick_Init();
 *              TIREMO_SysTick_DelayMs(500U);
 *              @endcode
 *
 * @note        Call TIREMO_SysTick_Init() once before using any delay function.
 * @note        TIREMO_SysTick_DelayMs() is blocking; the CPU waits in a busy loop.
 * @note        Override TIREMO_SysTick_OnTick1ms() (weak) for MQTT / app 1 ms work.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_SYSTICK_H_
#define TIREMO_SYSTICK_H_

#include <stdint.h>

/** @brief SysTick divider to generate a 1 ms tick from SystemCoreClock. */
#define TIREMO_SYSTICK_1MS_DIV      (1000U)

/**
 * @brief   Configure SysTick for 1 ms interrupts.
 * @details Must be called once at application startup before any delay call.
 */
void TIREMO_SysTick_Init(void);

/**
 * @brief       Blocking delay in milliseconds.
 * @param[in]   delayMs  Delay duration in milliseconds.
 * @note        Blocks the calling context until the delay expires.
 */
void TIREMO_SysTick_DelayMs(uint32_t delayMs);

/**
 * @brief   Free-running millisecond counter (wraps at UINT32_MAX).
 * @return  Milliseconds since TIREMO_SysTick_Init().
 */
uint32_t TIREMO_SysTick_GetMs(void);

/**
 * @brief   Optional 1 ms hook — weak default is empty.
 * @details Override in Tiremo_Process (e.g. MQTT port tick, mqtt_timer).
 */
void TIREMO_SysTick_OnTick1ms(void);

/**
 * @brief       Alias for TIREMO_SysTick_DelayMs (legacy MQTT cert helpers).
 * @param[in]   un32TimeMS  Delay in milliseconds.
 */
void SYSTICK_Wait(uint32_t un32TimeMS);

#endif /* TIREMO_SYSTICK_H_ */
