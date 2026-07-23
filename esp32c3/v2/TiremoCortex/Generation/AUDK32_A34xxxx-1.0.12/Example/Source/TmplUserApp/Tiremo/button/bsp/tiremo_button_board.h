/**
 *******************************************************************************
 * @file        tiremo_button_board.h
 * @brief       Board-specific button pin mapping
 *
 * @details     Maps logical button indices to PCU port/pin pairs for the
 *              AUDK32 evaluation board. Update this file when porting the
 *              library to a different board.
 *
 * @note        User button is active-low (PCU_PORT_LOW when pressed).
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_BUTTON_BOARD_H_
#define TIREMO_BUTTON_BOARD_H_

#include "hal_pcu.h"

/** @brief Number of buttons available on the board. */
#define TIREMO_BTN_COUNT                (1U)

#define TIREMO_BOARD_USER_BTN_PORT      PCU_ID_C
#define TIREMO_BOARD_USER_BTN_PIN       PCU_PIN_ID_9    /**< PC9 */

/** @brief NVIC priority for the user button port interrupt. */
#define TIREMO_BOARD_USER_BTN_IRQ_PRIO  (3U)

/** @brief Build PCU interrupt event mask for a given pin index. */
#define TIREMO_BTN_INTR_MASK(pin)       (0x3UL << ((uint32_t)(pin) * 2U))

#define TIREMO_BOARD_USER_BTN_INTR_MASK TIREMO_BTN_INTR_MASK(TIREMO_BOARD_USER_BTN_PIN)

/**
 * @brief Board pin descriptor for a single button.
 */
typedef struct
{
    PCU_ID_e port;      /**< PCU port group. */
    PCU_PIN_ID_e pin;   /**< Pin index within the port group. */
} TiremoBtnPin_t;

#endif /* TIREMO_BUTTON_BOARD_H_ */
