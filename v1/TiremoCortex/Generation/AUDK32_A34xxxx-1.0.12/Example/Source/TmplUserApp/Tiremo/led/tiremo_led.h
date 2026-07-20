/**
 *******************************************************************************
 * @file        tiremo_led.h
 * @brief       Tiremo LED library umbrella header
 *
 * @details     Single entry point for the Tiremo LED library.
 *
 *              Layer structure:
 *              - BSP  : Hardware control (init, on/off, toggle)
 *              - APP  : High-level patterns (blink, chase, wave)
 *
 *              Usage:
 *              @code
 *              #include "Tiremo/common/tiremo_systick.h"
 *              #include "Tiremo/led/tiremo_led.h"
 *
 *              TIREMO_SysTick_Init();
 *              TIREMO_LED_Init();
 *              TIREMO_LED_App_BlinkAll(500U, 500U);
 *              @endcode
 *
 * @note        Application layer functions are blocking and require SysTick.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_LED_H_
#define TIREMO_LED_H_

#include "bsp/tiremo_led_bsp.h"
#include "app/tiremo_led_app.h"

#endif /* TIREMO_LED_H_ */
