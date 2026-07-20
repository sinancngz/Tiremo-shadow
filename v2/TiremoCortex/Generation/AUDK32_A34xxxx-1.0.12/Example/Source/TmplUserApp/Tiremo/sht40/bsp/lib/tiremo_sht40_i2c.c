#include "tiremo_sht40_i2c.h"
#include "tiremo_sht40_common.h"
#include "tiremo_sht40_i2c_hal.h"

#define TIREMO_SHT40_CRC8_POLYNOMIAL (0x31U)
#define TIREMO_SHT40_CRC8_INIT       (0xFFU)

uint8_t tiremo_sht40_i2c_generate_crc(const uint8_t* data, uint16_t count) {
    uint16_t current_byte;
    uint8_t crc = TIREMO_SHT40_CRC8_INIT;
    uint8_t crc_bit;

    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= data[current_byte];
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1) ^ TIREMO_SHT40_CRC8_POLYNOMIAL);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

int8_t tiremo_sht40_i2c_check_crc(const uint8_t* data, uint16_t count, uint8_t checksum) {
    return (tiremo_sht40_i2c_generate_crc(data, count) == checksum) ?
           TIREMO_SHT40_NO_ERROR : TIREMO_SHT40_CRC_ERROR;
}

int16_t tiremo_sht40_i2c_write_data(uint8_t address, const uint8_t* data, uint16_t data_length) {
    return tiremo_sht40_i2c_hal_write(address, data, data_length);
}

int16_t tiremo_sht40_i2c_read_data_inplace(uint8_t address, uint8_t* buffer,
                                           uint16_t expected_data_length) {
    int16_t error;
    uint16_t i;
    uint16_t j;
    uint16_t size;

    if ((expected_data_length % TIREMO_SHT40_WORD_SIZE) != 0U) {
        return TIREMO_SHT40_BYTE_NUM_ERROR;
    }

    size = (uint16_t)((expected_data_length / TIREMO_SHT40_WORD_SIZE) *
                      (TIREMO_SHT40_WORD_SIZE + TIREMO_SHT40_CRC8_LEN));

    error = tiremo_sht40_i2c_hal_read(address, buffer, size);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }

    for (i = 0U, j = 0U; i < size; i += (TIREMO_SHT40_WORD_SIZE + TIREMO_SHT40_CRC8_LEN)) {
        error = tiremo_sht40_i2c_check_crc(&buffer[i], TIREMO_SHT40_WORD_SIZE,
                                           buffer[i + TIREMO_SHT40_WORD_SIZE]);
        if (error != TIREMO_SHT40_NO_ERROR) {
            return error;
        }
        buffer[j++] = buffer[i];
        buffer[j++] = buffer[i + 1U];
    }

    return TIREMO_SHT40_NO_ERROR;
}
