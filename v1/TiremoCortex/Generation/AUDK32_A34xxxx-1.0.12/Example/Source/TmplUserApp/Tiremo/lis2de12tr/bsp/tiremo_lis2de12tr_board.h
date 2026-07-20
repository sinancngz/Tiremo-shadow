/**
 *******************************************************************************
 * @file        tiremo_lis2de12tr_board.h
 * @brief       Board-specific LIS2DE12TR pin mapping
 *
 * @details     Maps LIS2DE12TR INT1/INT2 lines to PCU port/pin pairs.
 *              PA4 = INT1, PA5 = INT2 (configured in MCU Brew32).
 *
 * @note        GPIO interrupt mode: Edge, Both Edge, debounce enabled.
 *******************************************************************************
 */

#ifndef TIREMO_LIS2DE12TR_BOARD_H_
#define TIREMO_LIS2DE12TR_BOARD_H_

#include "hal_pcu.h"

#define TIREMO_LIS2DE12TR_INT1_PORT     PCU_ID_A
#define TIREMO_LIS2DE12TR_INT1_PIN      PCU_PIN_ID_4    /**< PA4 = LIS2DE12 INT1 */

#define TIREMO_LIS2DE12TR_INT2_PORT     PCU_ID_A
#define TIREMO_LIS2DE12TR_INT2_PIN      PCU_PIN_ID_5    /**< PA5 = LIS2DE12 INT2 */

/** @brief NVIC priority for LIS2DE12TR port interrupt (PA). */
#define TIREMO_LIS2DE12TR_IRQ_PRIO      (3U)

/** @brief Build PCU interrupt event mask for a given pin index. */
#define TIREMO_LIS2DE12TR_INTR_MASK(pin) (0x3UL << ((uint32_t)(pin) * 2U))

#define TIREMO_LIS2DE12TR_INT1_INTR_MASK \
    TIREMO_LIS2DE12TR_INTR_MASK(TIREMO_LIS2DE12TR_INT1_PIN)

#define TIREMO_LIS2DE12TR_INT2_INTR_MASK \
    TIREMO_LIS2DE12TR_INTR_MASK(TIREMO_LIS2DE12TR_INT2_PIN)

#endif /* TIREMO_LIS2DE12TR_BOARD_H_ */
