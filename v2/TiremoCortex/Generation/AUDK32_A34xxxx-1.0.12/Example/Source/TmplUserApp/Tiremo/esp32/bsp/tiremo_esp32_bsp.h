/**
 *******************************************************************************
 * @file        tiremo_esp32_bsp.h
 * @brief       Tiremo ESP32 board support package API
 *******************************************************************************
 */

#ifndef TIREMO_ESP32_BSP_H_
#define TIREMO_ESP32_BSP_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief       Power on ESP32 module after Brew pin/UART init.
 * @return      true on success, false on failure.
 * @note        Call after PRV_UART_Init() and TIREMO_SysTick_Init().
 *              PA12 and UART2 are configured in MCU Brew32.
 */
bool TIREMO_ESP32_BSP_Init(void);

/**
 * @brief       Power on ESP32 module (PA12).
 * @return      None
 */
void TIREMO_ESP32_BSP_PowerOn(void);

/**
 * @brief       Power off ESP32 module (PA12).
 * @return      None
 */
void TIREMO_ESP32_BSP_PowerOff(void);

/**
 * @brief       Power-cycle ESP32 module.
 * @return      None
 */
void TIREMO_ESP32_BSP_PowerCycle(void);

/**
 * @brief       Send raw bytes on ESP32 UART.
 * @param[in]   data    TX buffer.
 * @param[in]   length  Number of bytes.
 * @return      true on success, false on failure.
 */
bool TIREMO_ESP32_BSP_UartTransmit(const uint8_t* data, uint32_t length);

/**
 * @brief       Receive bytes from ESP32 UART.
 * @param[out]  data       RX buffer.
 * @param[in]   maxLength  Maximum bytes to read.
 * @return      true on success, false on failure.
 */
bool TIREMO_ESP32_BSP_UartReceive(uint8_t* data, uint32_t maxLength);

/**
 * @brief       Send AT command and read response.
 * @param[in]   command    Null-terminated command (e.g. "AT").
 * @param[out]  response   Response buffer.
 * @param[in]   respSize   Response buffer size.
 * @return      true if response contains "OK", false otherwise.
 */
bool TIREMO_ESP32_BSP_SendAtCommand(const char* command, char* response, uint16_t respSize);

#endif /* TIREMO_ESP32_BSP_H_ */
