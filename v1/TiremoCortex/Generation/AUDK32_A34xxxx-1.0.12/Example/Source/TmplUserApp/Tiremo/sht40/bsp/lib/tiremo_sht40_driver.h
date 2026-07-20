/**
 *******************************************************************************
 * @file        tiremo_sht40_driver.h
 * @brief       Tiremo SHT40 sensor command API
 *
 * @details     Exposes full SHT40 command set for measurements, heater modes,
 *              serial number read, and soft reset.
 *******************************************************************************
 */

#ifndef TIREMO_SHT40_DRIVER_H_
#define TIREMO_SHT40_DRIVER_H_

#include <stdint.h>

/**
 * @brief       Single-shot measurement with high repeatability (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_measure_high_precision_ticks(uint16_t* temperature_ticks,
                                                  uint16_t* humidity_ticks);

/**
 * @brief       Single-shot measurement with high repeatability (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_measure_high_precision(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Single-shot measurement with medium repeatability (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_measure_medium_precision_ticks(uint16_t* temperature_ticks,
                                                    uint16_t* humidity_ticks);

/**
 * @brief       Single-shot measurement with medium repeatability (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_measure_medium_precision(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Single-shot measurement with lowest repeatability (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_measure_lowest_precision_ticks(uint16_t* temperature_ticks,
                                                    uint16_t* humidity_ticks);

/**
 * @brief       Single-shot measurement with lowest repeatability (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_measure_lowest_precision(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Highest heater power for 1 second (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_highest_heater_power_long_ticks(uint16_t* temperature_ticks,
                                                               uint16_t* humidity_ticks);

/**
 * @brief       Highest heater power for 1 second (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_highest_heater_power_long(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Highest heater power for 0.1 second (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_highest_heater_power_short_ticks(uint16_t* temperature_ticks,
                                                                uint16_t* humidity_ticks);

/**
 * @brief       Highest heater power for 0.1 second (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_highest_heater_power_short(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Medium heater power for 1 second (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_medium_heater_power_long_ticks(uint16_t* temperature_ticks,
                                                              uint16_t* humidity_ticks);

/**
 * @brief       Medium heater power for 1 second (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_medium_heater_power_long(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Medium heater power for 0.1 second (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_medium_heater_power_short_ticks(uint16_t* temperature_ticks,
                                                               uint16_t* humidity_ticks);

/**
 * @brief       Medium heater power for 0.1 second (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_medium_heater_power_short(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Lowest heater power for 1 second (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_lowest_heater_power_long_ticks(uint16_t* temperature_ticks,
                                                              uint16_t* humidity_ticks);

/**
 * @brief       Lowest heater power for 1 second (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_lowest_heater_power_long(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Lowest heater power for 0.1 second (raw ticks).
 * @param[out]  temperature_ticks  Temperature raw ticks.
 * @param[out]  humidity_ticks     Humidity raw ticks.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_lowest_heater_power_short_ticks(uint16_t* temperature_ticks,
                                                               uint16_t* humidity_ticks);

/**
 * @brief       Lowest heater power for 0.1 second (milli-units).
 * @param[out]  temperature  Temperature in milli-degree Celsius.
 * @param[out]  humidity     Humidity in milli-percent RH.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_activate_lowest_heater_power_short(int32_t* temperature, int32_t* humidity);

/**
 * @brief       Read unique SHT40 serial number.
 * @param[out]  serial_number  Sensor unique serial number.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_serial_number(uint32_t* serial_number);

/**
 * @brief       Perform SHT40 soft reset.
 * @param[in]   None
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_soft_reset(void);

#endif /* TIREMO_SHT40_DRIVER_H_ */
