/**
 *******************************************************************************
 * @file        tiremo_sht40_app.h
 * @brief       Tiremo SHT40 application layer API
 *
 * @details     Contains high-level application routines that use SHT40 BSP for
 *              periodic measurement, logging, and LED threshold behavior.
 *******************************************************************************
 */

#ifndef TIREMO_SHT40_APP_H_
#define TIREMO_SHT40_APP_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief       Initialize SHT40 application layer.
 * @param[in]   None
 * @return      true on success, false on failure.
 */
bool TIREMO_SHT40_App_Init(void);

/**
 * @brief       Read one SHT40 sample through BSP.
 * @param[out]  temperature_mC  Temperature in milli-degree Celsius.
 * @param[out]  humidity_mRH    Relative humidity in milli-percent RH.
 * @return      true on success, false on failure.
 */
bool TIREMO_SHT40_App_Read(int32_t* temperature_mC, int32_t* humidity_mRH);

#endif /* TIREMO_SHT40_APP_H_ */
