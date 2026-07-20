/**
 *******************************************************************************
 * @file        tiremo_led_bsp.h
 * @brief       Tiremo LED board support package (BSP) API
 *
 * @details     Hardware abstraction layer for on-board LEDs. All functions
 *              operate on logical LED identifiers (TiremoLedId_e) and hide
 *              the underlying GPIO/PCU details.
 *
 *              Typical usage:
 *              @code
 *              TIREMO_LED_Init();
 *              TIREMO_LED_On(TIREMO_LED_1);
 *              TIREMO_LED_Toggle(TIREMO_LED_3);
 *              TIREMO_LED_AllOff();
 *              @endcode
 *
 * @note        Invalid ledId values are silently ignored by control functions.
 * @note        TIREMO_LED_GetState() returns false for invalid IDs.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_LED_BSP_H_
#define TIREMO_LED_BSP_H_

#include <stdbool.h>
#include <stdint.h>

#include "tiremo_led_board.h"

/**
 * @brief Logical LED identifier.
 * @note  Values map 1:1 to the board pin table in tiremo_led_board.h.
 */
typedef enum
{
    TIREMO_LED_1 = 0U,
    TIREMO_LED_2,
    TIREMO_LED_3,
    TIREMO_LED_4,
    TIREMO_LED_5,
    TIREMO_LED_6,
    TIREMO_LED_7,
    TIREMO_LED_8,
    TIREMO_LED_9,
    TIREMO_LED_10
} TiremoLedId_e;

/**
 * @brief   Initialize all LEDs and turn them off.
 */
void TIREMO_LED_Init(void);

/**
 * @brief       Turn on a single LED.
 * @param[in]   ledId  Target LED identifier.
 */
void TIREMO_LED_On(TiremoLedId_e ledId);

/**
 * @brief       Turn off a single LED.
 * @param[in]   ledId  Target LED identifier.
 */
void TIREMO_LED_Off(TiremoLedId_e ledId);

/**
 * @brief       Toggle a single LED.
 * @param[in]   ledId  Target LED identifier.
 */
void TIREMO_LED_Toggle(TiremoLedId_e ledId);

/**
 * @brief       Set the on/off state of a single LED.
 * @param[in]   ledId  Target LED identifier.
 * @param[in]   isOn   true to turn on, false to turn off.
 */
void TIREMO_LED_SetState(TiremoLedId_e ledId, bool isOn);

/**
 * @brief       Get the logical on/off state of a single LED.
 * @param[in]   ledId  Target LED identifier.
 * @return      true if the LED is on, false if off or if ledId is invalid.
 */
bool TIREMO_LED_GetState(TiremoLedId_e ledId);

/** @brief Turn on all board LEDs. */
void TIREMO_LED_AllOn(void);

/** @brief Turn off all board LEDs. */
void TIREMO_LED_AllOff(void);

#endif /* TIREMO_LED_BSP_H_ */
