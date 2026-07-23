#include "tiremo_sht40_common.h"

uint16_t tiremo_sht40_common_bytes_to_uint16(const uint8_t* bytes) {
    return (uint16_t)bytes[0] << 8 | (uint16_t)bytes[1];
}

uint32_t tiremo_sht40_common_bytes_to_uint32(const uint8_t* bytes) {
    return (uint32_t)bytes[0] << 24 | (uint32_t)bytes[1] << 16 |
           ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

int16_t tiremo_sht40_common_bytes_to_int16(const uint8_t* bytes) {
    return (int16_t)tiremo_sht40_common_bytes_to_uint16(bytes);
}

int32_t tiremo_sht40_common_bytes_to_int32(const uint8_t* bytes) {
    return (int32_t)tiremo_sht40_common_bytes_to_uint32(bytes);
}

float tiremo_sht40_common_bytes_to_float(const uint8_t* bytes) {
    union {
        uint32_t u32_value;
        float float32;
    } tmp;

    tmp.u32_value = tiremo_sht40_common_bytes_to_uint32(bytes);
    return tmp.float32;
}

void tiremo_sht40_common_uint32_to_bytes(uint32_t value, uint8_t* bytes) {
    bytes[0] = (uint8_t)(value >> 24);
    bytes[1] = (uint8_t)(value >> 16);
    bytes[2] = (uint8_t)(value >> 8);
    bytes[3] = (uint8_t)value;
}

void tiremo_sht40_common_uint16_to_bytes(uint16_t value, uint8_t* bytes) {
    bytes[0] = (uint8_t)(value >> 8);
    bytes[1] = (uint8_t)value;
}

void tiremo_sht40_common_int32_to_bytes(int32_t value, uint8_t* bytes) {
    bytes[0] = (uint8_t)(value >> 24);
    bytes[1] = (uint8_t)(value >> 16);
    bytes[2] = (uint8_t)(value >> 8);
    bytes[3] = (uint8_t)value;
}

void tiremo_sht40_common_int16_to_bytes(int16_t value, uint8_t* bytes) {
    bytes[0] = (uint8_t)(value >> 8);
    bytes[1] = (uint8_t)value;
}

void tiremo_sht40_common_float_to_bytes(float value, uint8_t* bytes) {
    union {
        uint32_t u32_value;
        float float32;
    } tmp;
    tmp.float32 = value;
    tiremo_sht40_common_uint32_to_bytes(tmp.u32_value, bytes);
}

void tiremo_sht40_common_copy_bytes(const uint8_t* source, uint8_t* destination,
                                    uint16_t data_length) {
    uint16_t i;
    for (i = 0; i < data_length; i++) {
        destination[i] = source[i];
    }
}

uint16_t tiremo_sht40_bytes_to_uint16(const uint8_t* bytes) {
    return tiremo_sht40_common_bytes_to_uint16(bytes);
}

uint32_t tiremo_sht40_bytes_to_uint32(const uint8_t* bytes) {
    return tiremo_sht40_common_bytes_to_uint32(bytes);
}
