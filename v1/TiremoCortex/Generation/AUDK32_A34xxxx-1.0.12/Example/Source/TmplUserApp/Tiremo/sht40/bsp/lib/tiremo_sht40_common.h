/**
 *******************************************************************************
 * @file        tiremo_sht40_common.h
 * @brief       Common utility conversions for Tiremo SHT40 library
 *
 * @details     Defines byte conversion helpers, common constants, and utility
 *              routines shared by the SHT40 I2C/driver layers.
 *******************************************************************************
 */

#ifndef TIREMO_SHT40_COMMON_H_
#define TIREMO_SHT40_COMMON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIREMO_SHT40_NO_ERROR             (0)
#define TIREMO_SHT40_NOT_IMPLEMENTED      (31)

#define TIREMO_SHT40_WORD_SIZE            (2U)
#define TIREMO_SHT40_CRC8_LEN             (1U)
#define TIREMO_SHT40_MAX_BUFFER_WORDS     (32U)

/**
 * @brief       Convert MSB-first bytes to uint16_t.
 * @param[in]   bytes  Input byte pointer.
 * @return      Converted uint16_t value.
 */
uint16_t tiremo_sht40_common_bytes_to_uint16(const uint8_t* bytes);

/**
 * @brief       Convert MSB-first bytes to uint32_t.
 * @param[in]   bytes  Input byte pointer.
 * @return      Converted uint32_t value.
 */
uint32_t tiremo_sht40_common_bytes_to_uint32(const uint8_t* bytes);

/**
 * @brief       Convert MSB-first bytes to int16_t.
 * @param[in]   bytes  Input byte pointer.
 * @return      Converted int16_t value.
 */
int16_t tiremo_sht40_common_bytes_to_int16(const uint8_t* bytes);

/**
 * @brief       Convert MSB-first bytes to int32_t.
 * @param[in]   bytes  Input byte pointer.
 * @return      Converted int32_t value.
 */
int32_t tiremo_sht40_common_bytes_to_int32(const uint8_t* bytes);

/**
 * @brief       Convert MSB-first bytes to float.
 * @param[in]   bytes  Input byte pointer.
 * @return      Converted float value.
 */
float tiremo_sht40_common_bytes_to_float(const uint8_t* bytes);

/**
 * @brief       Convert uint32_t to MSB-first byte array.
 * @param[in]   value  Source value.
 * @param[out]  bytes  Output byte array.
 * @return      None
 */
void tiremo_sht40_common_uint32_to_bytes(uint32_t value, uint8_t* bytes);

/**
 * @brief       Convert uint16_t to MSB-first byte array.
 * @param[in]   value  Source value.
 * @param[out]  bytes  Output byte array.
 * @return      None
 */
void tiremo_sht40_common_uint16_to_bytes(uint16_t value, uint8_t* bytes);

/**
 * @brief       Convert int32_t to MSB-first byte array.
 * @param[in]   value  Source value.
 * @param[out]  bytes  Output byte array.
 * @return      None
 */
void tiremo_sht40_common_int32_to_bytes(int32_t value, uint8_t* bytes);

/**
 * @brief       Convert int16_t to MSB-first byte array.
 * @param[in]   value  Source value.
 * @param[out]  bytes  Output byte array.
 * @return      None
 */
void tiremo_sht40_common_int16_to_bytes(int16_t value, uint8_t* bytes);

/**
 * @brief       Convert float to MSB-first byte array.
 * @param[in]   value  Source value.
 * @param[out]  bytes  Output byte array.
 * @return      None
 */
void tiremo_sht40_common_float_to_bytes(float value, uint8_t* bytes);

/**
 * @brief       Copy bytes from source to destination.
 * @param[in]   source       Source byte array.
 * @param[out]  destination  Destination byte array.
 * @param[in]   data_length  Number of bytes to copy.
 * @return      None
 */
void tiremo_sht40_common_copy_bytes(const uint8_t* source, uint8_t* destination,
                                    uint16_t data_length);

/* Backward-compatible aliases used by current BSP code. */
uint16_t tiremo_sht40_bytes_to_uint16(const uint8_t* bytes);
uint32_t tiremo_sht40_bytes_to_uint32(const uint8_t* bytes);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_SHT40_COMMON_H_ */
