/**
 *******************************************************************************
 * @file        tiremo_sht40_bsp.h
 * @brief       Tiremo SHT40 board support package API
 *
 * @details     Provides hardware-facing routines for SHT40 initialization and
 *              high precision temperature/humidity measurement.
 *******************************************************************************
 */

#ifndef TIREMO_SHT40_BSP_H_
#define TIREMO_SHT40_BSP_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief       Initialize SHT40 BSP and underlying I2C path.
 * @param[in]   None
 * @return      true on success, false on failure.
 */
bool TIREMO_SHT40_BSP_Init(void);

/**
 * @brief       Read one high precision SHT40 sample.
 * @param[out]  temperature_mC   Temperature in milli-degree Celsius.
 * @param[out]  humidity_mRH     Relative humidity in milli-percent RH.
 * @return      true on success, false on failure.
 */
bool TIREMO_SHT40_BSP_ReadHighPrecision(int32_t* temperature_mC, int32_t* humidity_mRH);

/**
 * @brief       Deinitialize SHT40 BSP resources.
 * @param[in]   None
 * @return      None
 */
void TIREMO_SHT40_BSP_Deinit(void);

#endif /* TIREMO_SHT40_BSP_H_ */
