/**
 *******************************************************************************
 * @file        tiremo_sht40_i2c_hal.h
 * @brief       Tiremo SHT40 I2C hardware abstraction layer API
 *
 * @details     Declares platform I2C access and delay hooks used by the SHT40
 *              low-level I2C helper routines.
 *******************************************************************************
 */

#ifndef TIREMO_SHT40_I2C_HAL_H_
#define TIREMO_SHT40_I2C_HAL_H_

#include <stdint.h>

/**
 * @brief       Select active I2C bus index.
 * @param[in]   bus_idx  I2C bus index.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_i2c_hal_select_bus(uint8_t bus_idx);

/**
 * @brief       Initialize platform-specific I2C resources.
 * @param[in]   None
 * @return      None
 */
void tiremo_sht40_i2c_hal_init(void);

/**
 * @brief       Release platform-specific I2C resources.
 * @param[in]   None
 * @return      None
 */
void tiremo_sht40_i2c_hal_free(void);

/**
 * @brief       Read bytes from 7-bit I2C address.
 * @param[in]   address  7-bit sensor I2C address.
 * @param[out]  data     Destination buffer.
 * @param[in]   count    Number of bytes to read.
 * @return      0 on success, error code otherwise.
 */
int8_t tiremo_sht40_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count);

/**
 * @brief       Write bytes to 7-bit I2C address.
 * @param[in]   address  7-bit sensor I2C address.
 * @param[in]   data     Source buffer.
 * @param[in]   count    Number of bytes to write.
 * @return      0 on success, error code otherwise.
 */
int8_t tiremo_sht40_i2c_hal_write(uint8_t address, const uint8_t* data, uint16_t count);

/**
 * @brief       Blocking delay in microseconds.
 * @param[in]   useconds  Delay duration in microseconds.
 * @note        Blocks the calling context until the delay expires.
 */
void tiremo_sht40_i2c_hal_sleep_usec(uint32_t useconds);

#endif /* TIREMO_SHT40_I2C_HAL_H_ */
