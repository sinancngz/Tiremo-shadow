/**
 *******************************************************************************
 * @file        tiremo_led_board.h
 * @brief       Board-specific LED pin mapping
 *
 * @details     Maps logical LED indices to PCU port/pin pairs for the
 *              AUDK32 evaluation board. Update this file when porting the
 *              library to a different board.
 *
 * @note        LEDs on the AUDK32 board are active-low (driven LOW to turn on).
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_LED_BOARD_H_
#define TIREMO_LED_BOARD_H_

#include "hal_pcu.h"

/** @brief Number of LEDs available on the board. */
#define TIREMO_LED_COUNT            (10U)

#define TIREMO_BOARD_LED1_PORT      PCU_ID_B
#define TIREMO_BOARD_LED1_PIN       PCU_PIN_ID_4    /**< PB4  */
#define TIREMO_BOARD_LED2_PORT      PCU_ID_B
#define TIREMO_BOARD_LED2_PIN       PCU_PIN_ID_9    /**< PB9  */
#define TIREMO_BOARD_LED3_PORT      PCU_ID_F
#define TIREMO_BOARD_LED3_PIN       PCU_PIN_ID_7    /**< PF7  */
#define TIREMO_BOARD_LED4_PORT      PCU_ID_B
#define TIREMO_BOARD_LED4_PIN       PCU_PIN_ID_5    /**< PB5  */
#define TIREMO_BOARD_LED5_PORT      PCU_ID_B
#define TIREMO_BOARD_LED5_PIN       PCU_PIN_ID_10   /**< PB10 */
#define TIREMO_BOARD_LED6_PORT      PCU_ID_B
#define TIREMO_BOARD_LED6_PIN       PCU_PIN_ID_11   /**< PB11 */
#define TIREMO_BOARD_LED7_PORT      PCU_ID_B
#define TIREMO_BOARD_LED7_PIN       PCU_PIN_ID_12   /**< PB12 */
#define TIREMO_BOARD_LED8_PORT      PCU_ID_B
#define TIREMO_BOARD_LED8_PIN       PCU_PIN_ID_13   /**< PB13 */
#define TIREMO_BOARD_LED9_PORT      PCU_ID_B
#define TIREMO_BOARD_LED9_PIN       PCU_PIN_ID_14   /**< PB14 */
#define TIREMO_BOARD_LED10_PORT     PCU_ID_B
#define TIREMO_BOARD_LED10_PIN      PCU_PIN_ID_15   /**< PB15 */

/**
 * @brief Board pin descriptor for a single LED.
 */
typedef struct
{
    PCU_ID_e port;      /**< PCU port group. */
    PCU_PIN_ID_e pin;   /**< Pin index within the port group. */
} TiremoLedPin_t;

#endif /* TIREMO_LED_BOARD_H_ */
