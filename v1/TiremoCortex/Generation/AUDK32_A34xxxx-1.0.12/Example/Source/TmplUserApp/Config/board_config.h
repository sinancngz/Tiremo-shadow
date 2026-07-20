/**
 * @file    board_config.h
 * @brief   Tiremo v2 board pin map and UART selection
 *
 * If you use a different PCB, update only the pins in this file.
 * LEDs are active-low: SET = off, CLEAR = on.
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "hal_pcu.h"
#include "hal_uart.h"

/* =========================================================================
 * LEDs (active-low) — test and status indicators
 * ========================================================================= */

#define BOARD_LED1_PORT             PCU_ID_B
#define BOARD_LED1_PIN              PCU_PIN_ID_4    /* PB4  — debug init OK */
#define BOARD_LED2_PORT             PCU_ID_B
#define BOARD_LED2_PIN              PCU_PIN_ID_9    /* PB9  */
#define BOARD_LED3_PORT             PCU_ID_F
#define BOARD_LED3_PIN              PCU_PIN_ID_7    /* PF7  */
#define BOARD_LED4_PORT             PCU_ID_B
#define BOARD_LED4_PIN              PCU_PIN_ID_5    /* PB5  */
#define BOARD_LED5_PORT             PCU_ID_B
#define BOARD_LED5_PIN              PCU_PIN_ID_10   /* PB10 */
#define BOARD_LED6_PORT             PCU_ID_B
#define BOARD_LED6_PIN              PCU_PIN_ID_11   /* PB11 */
#define BOARD_LED7_PORT             PCU_ID_B
#define BOARD_LED7_PIN              PCU_PIN_ID_12   /* PB12 */
#define BOARD_LED8_PORT             PCU_ID_B
#define BOARD_LED8_PIN              PCU_PIN_ID_13   /* PB13 */
#define BOARD_LED9_PORT             PCU_ID_B
#define BOARD_LED9_PIN              PCU_PIN_ID_14   /* PB14 */
#define BOARD_LED10_PORT            PCU_ID_B
#define BOARD_LED10_PIN             PCU_PIN_ID_15   /* PB15 */

/* Logical LED names — used by mqtt_port_abov.c and prv_user_code */
#define BOARD_LED_MQTT_TX_PORT      BOARD_LED6_PORT
#define BOARD_LED_MQTT_TX_PIN       BOARD_LED6_PIN
/* Blinks briefly when MQTT data is transmitted (TX blink). */

#define BOARD_LED_WIFI_STATUS_PORT  BOARD_LED7_PORT
#define BOARD_LED_WIFI_STATUS_PIN   BOARD_LED7_PIN
/* Stays on when ESP32 WiFi is connected. */

#define BOARD_LED_MQTT_CONN_PORT    BOARD_LED8_PORT
#define BOARD_LED_MQTT_CONN_PIN     BOARD_LED8_PIN
/* Stays on when connected to the MQTT broker. */

#define BOARD_LED_MQTT_RX_PORT      BOARD_LED9_PORT
#define BOARD_LED_MQTT_RX_PIN       BOARD_LED9_PIN
/* Blinks when an MQTT message is received (RX blink). */

/* =========================================================================
 * I2C — SHT40, LIS2DE12 sensors
 * ========================================================================= */

#define BOARD_I2C_SCL_PORT          PCU_ID_B
#define BOARD_I2C_SCL_PIN           PCU_PIN_ID_6     /* PB6 */
#define BOARD_I2C_SDA_PORT          PCU_ID_B
#define BOARD_I2C_SDA_PIN           PCU_PIN_ID_7     /* PB7 */
#define BOARD_I2C_ALT_FUNCTION      PCU_ALT_1
/* I2C2 alternate function. Used in prv_user_code GPIO_Config_Alt(). */

/* =========================================================================
 * UART: ESP32-C3 AT module
 * ========================================================================= */

#define BOARD_ESP32_UART_ID         UART_ID_2
/* HAL UART instance. mqtt_port_abov.c sends AT commands on this UART. */

#define BOARD_ESP32_UART_PORT       PCU_ID_A
#define BOARD_ESP32_UART_RX_PIN     PCU_PIN_ID_8     /* PA8 = ESP32 TX -> MCU RX */
#define BOARD_ESP32_UART_TX_PIN     PCU_PIN_ID_9     /* PA9 = ESP32 RX <- MCU TX */
#define BOARD_ESP32_UART_ALT        PCU_ALT_1

#define BOARD_ESP32_PWR_PORT        PCU_ID_A
#define BOARD_ESP32_PWR_PIN         PCU_PIN_ID_12    /* PA12 = ESP32 supply enable */
/* HIGH = powered, LOW = off. Swap ON/OFF if your board is active-low. */
#define BOARD_ESP32_PWR_ON          PCU_OUTPUT_BIT_SET
#define BOARD_ESP32_PWR_OFF         PCU_OUTPUT_BIT_CLEAR
#define BOARD_ESP32_PWR_OFF_MS      500U             /* ms with supply off during cycle */
#define BOARD_ESP32_PWR_BOOT_MS     3000U            /* ms after power-on before AT test */

/* =========================================================================
 * UART: SLM320 4G modem + power pins
 * ========================================================================= */

#define BOARD_MODEM_UART_ID         UART_ID_1
/* Modem AT commands use this UART. */

#define BOARD_MODEM_UART_PORT       PCU_ID_A
#define BOARD_MODEM_UART_RX_PIN     PCU_PIN_ID_10    /* PA10 */
#define BOARD_MODEM_UART_TX_PIN     PCU_PIN_ID_11    /* PA11 */
#define BOARD_MODEM_UART_ALT        PCU_ALT_1

#define BOARD_MODEM_PWRKEY_PORT     PCU_ID_A
#define BOARD_MODEM_PWRKEY_PIN      PCU_PIN_ID_7
/* PWRKEY: hold LOW >= 1 s to power on the modem. */

#define BOARD_MODEM_PWR_PORT        PCU_ID_C
#define BOARD_MODEM_PWR_PIN         PCU_PIN_ID_4
/* Module power supply enable pin. */

/* =========================================================================
 * User button — PC9
 * ========================================================================= */

#define BOARD_USER_BTN_PORT         PCU_ID_C
#define BOARD_USER_BTN_PIN          PCU_PIN_ID_9    /* PC9 */
#define BOARD_USER_BTN_ACTIVE_LEVEL PCU_PORT_LOW    /* pressed = LOW (internal pull-up) */
#define BOARD_USER_BTN_IRQ_PRIO     3U

#endif /* BOARD_CONFIG_H */
