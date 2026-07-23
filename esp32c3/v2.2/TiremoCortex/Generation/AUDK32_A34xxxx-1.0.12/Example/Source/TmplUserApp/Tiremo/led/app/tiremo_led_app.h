/**
 *******************************************************************************
 * @file        tiremo_led_app.h
 * @brief       Tiremo LED application layer API
 *
 * @details     High-level LED patterns built on top of the BSP layer.
 *              All functions are blocking and use TIREMO_SysTick_DelayMs()
 *              for timing.
 *
 *              Prerequisites:
 *              - TIREMO_SysTick_Init() must be called before any App function.
 *              - TIREMO_LED_Init() should be called to ensure a known state.
 *
 * @note        Each function runs one complete pattern cycle and then returns.
 *              Call repeatedly from a loop for continuous animation.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_LED_APP_H_
#define TIREMO_LED_APP_H_

#include <stdint.h>

#include "../bsp/tiremo_led_bsp.h"

/**
 * @brief       Blink a single LED once (on, then off).
 * @param[in]   ledId   Target LED identifier.
 * @param[in]   onMs    Time the LED stays on, in milliseconds.
 * @param[in]   offMs   Time the LED stays off, in milliseconds.
 */
void TIREMO_LED_App_Blink(TiremoLedId_e ledId, uint32_t onMs, uint32_t offMs);

/**
 * @brief       Blink all LEDs together once.
 * @param[in]   onMs    Time all LEDs stay on, in milliseconds.
 * @param[in]   offMs   Time all LEDs stay off, in milliseconds.
 */
void TIREMO_LED_App_BlinkAll(uint32_t onMs, uint32_t offMs);

/**
 * @brief       Toggle a single LED once.
 * @param[in]   ledId   Target LED identifier.
 * @note        Does not include a delay; add TIREMO_SysTick_DelayMs() if needed.
 */
void TIREMO_LED_App_Toggle(TiremoLedId_e ledId);

/**
 * @brief       Running-light effect across all LEDs.
 * @param[in]   stepMs  Delay per LED step, in milliseconds.
 * @details     Turns off all LEDs, then lights each one sequentially
 *              (LED1 -> LED10) before returning.
 */
void TIREMO_LED_App_Chase(uint32_t stepMs);

/**
 * @brief       Wave effect across all LEDs.
 * @param[in]   stepMs  Delay per LED step, in milliseconds.
 * @details     Lights LEDs sequentially (LED1 -> LED10), then turns them
 *              off in the same order before returning.
 */
void TIREMO_LED_App_Wave(uint32_t stepMs);

#endif /* TIREMO_LED_APP_H_ */
