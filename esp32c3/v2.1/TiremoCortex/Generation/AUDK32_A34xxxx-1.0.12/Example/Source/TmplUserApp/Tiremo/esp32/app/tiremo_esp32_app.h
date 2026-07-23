/**
 *******************************************************************************
 * @file        tiremo_esp32_app.h
 * @brief       Tiremo ESP32 application layer API
 *******************************************************************************
 */

#ifndef TIREMO_ESP32_APP_H_
#define TIREMO_ESP32_APP_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief       Initialize ESP32 application layer.
 * @return      true on success, false on failure.
 */
bool TIREMO_ESP32_App_Init(void);

/**
 * @brief       Send "AT" and check for OK response.
 * @return      true if OK received, false otherwise.
 */
bool TIREMO_ESP32_App_RunAtTest(void);

/**
 * @brief       AT test with power-cycle recovery on failure.
 * @return      true if OK received, false after all retries.
 */
bool TIREMO_ESP32_App_RunAtTestWithRecovery(void);

#endif /* TIREMO_ESP32_APP_H_ */
