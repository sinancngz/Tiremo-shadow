/**
 *******************************************************************************
 * @file        tiremo_sht40_i2c.h
 * @brief       Tiremo SHT40 I2C helper API
 *
 * @details     Provides CRC and framed read/write helper functions used by the
 *              SHT40 driver implementation.
 *******************************************************************************
 */

#ifndef TIREMO_SHT40_I2C_H_
#define TIREMO_SHT40_I2C_H_

#include <stdint.h>

#define TIREMO_SHT40_CRC_ERROR       (1)
#define TIREMO_SHT40_I2C_BUS_ERROR   (2)
#define TIREMO_SHT40_I2C_NACK_ERROR  (3)
#define TIREMO_SHT40_BYTE_NUM_ERROR  (4)

/**
 * @brief       Calculate CRC8 over given data bytes.
 * @param[in]   data   Data buffer.
 * @param[in]   count  Number of bytes.
 * @return      Calculated CRC8 value.
 */
uint8_t tiremo_sht40_i2c_generate_crc(const uint8_t* data, uint16_t count);

/**
 * @brief       Check CRC8 of one data chunk.
 * @param[in]   data      Data buffer.
 * @param[in]   count     Number of bytes.
 * @param[in]   checksum  Expected CRC8.
 * @return      0 on success, error code otherwise.
 */
int8_t tiremo_sht40_i2c_check_crc(const uint8_t* data, uint16_t count, uint8_t checksum);

/**
 * @brief       Write raw bytes to sensor I2C address.
 * @param[in]   address      7-bit sensor I2C address.
 * @param[in]   data         Source buffer.
 * @param[in]   data_length  Number of bytes to write.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_i2c_write_data(uint8_t address, const uint8_t* data, uint16_t data_length);

/**
 * @brief       Read sensor bytes and strip CRC bytes in-place.
 * @param[in]   address               7-bit sensor I2C address.
 * @param[out]  buffer                Destination buffer.
 * @param[in]   expected_data_length  Expected data length without CRC.
 * @return      0 on success, error code otherwise.
 */
int16_t tiremo_sht40_i2c_read_data_inplace(uint8_t address, uint8_t* buffer,
                                           uint16_t expected_data_length);

#endif /* TIREMO_SHT40_I2C_H_ */
