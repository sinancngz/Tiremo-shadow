/**
 *******************************************************************************
 * @file        tiremo_esp32_board.h
 * @brief       Board-specific ESP32-C3 pin and UART mapping
 *
 * @details     Matches Examples/Tiremo/config/board_config.h (Tiremo v2 board).
 *              ESP32 AT link: UART2 on PA8 (MCU RX) / PA9 (MCU TX).
 *              Power enable: PA12.
 *
 * @note        Pin mux and GPIO mode: MCU Brew32 (user_pin_config).
 *              UART init: MCU Brew32 (user_uart.c).
 *******************************************************************************
 */

#ifndef TIREMO_ESP32_BOARD_H_
#define TIREMO_ESP32_BOARD_H_

#include "hal_uart.h"
#include "hal_pcu.h"

/** @brief ESP32 AT UART instance (MCU UART2). */
#define TIREMO_ESP32_UART_ID            UART_ID_2

/** @brief ESP32 module power enable pin (PA12). */
#define TIREMO_ESP32_PWR_PORT           PCU_ID_A
#define TIREMO_ESP32_PWR_PIN            PCU_PIN_ID_12

/** @brief Power enable active level. */
#define TIREMO_ESP32_PWR_ON             PCU_OUTPUT_BIT_SET
#define TIREMO_ESP32_PWR_OFF            PCU_OUTPUT_BIT_CLEAR

/** @brief Power timing (ms) — same as Tiremo board_config.h. */
#define TIREMO_ESP32_PWR_OFF_MS         (500U)
#define TIREMO_ESP32_PWR_BOOT_MS        (3000U)

/** @brief AT test retry limits. */
#define TIREMO_ESP32_AT_MAX_ATTEMPTS    (10U)
#define TIREMO_ESP32_AT_POWER_CYCLE_MAX (3U)

/** @brief Default AT response read size. */
#define TIREMO_ESP32_AT_RX_LEN          (20U)

#endif /* TIREMO_ESP32_BOARD_H_ */
