#include "tiremo_sht40_driver.h"
#include "tiremo_sht40_common.h"
#include "tiremo_sht40_i2c.h"
#include "tiremo_sht40_i2c_hal.h"

#define TIREMO_SHT40_I2C_ADDRESS (0x44U)

static int32_t tiremo_sht40_convert_ticks_to_celsius(uint16_t ticks) {
    return ((21875 * (int32_t)ticks) >> 13) - 45000;
}

static int32_t tiremo_sht40_convert_ticks_to_percent_rh(uint16_t ticks) {
    return ((15625 * (int32_t)ticks) >> 13) - 6000;
}

static int16_t tiremo_sht40_read_measurement(uint8_t cmd, uint32_t delay_us,
                                             uint16_t* temperature_ticks, uint16_t* humidity_ticks) {
    int16_t error;
    uint8_t buffer[6];
    uint16_t offset = 0U;

    buffer[offset++] = cmd;
    error = tiremo_sht40_i2c_write_data(TIREMO_SHT40_I2C_ADDRESS, &buffer[0], offset);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }

    tiremo_sht40_i2c_hal_sleep_usec(delay_us);
    error = tiremo_sht40_i2c_read_data_inplace(TIREMO_SHT40_I2C_ADDRESS, &buffer[0], 4U);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }

    *temperature_ticks = tiremo_sht40_bytes_to_uint16(&buffer[0]);
    *humidity_ticks = tiremo_sht40_bytes_to_uint16(&buffer[2]);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_measure_high_precision_ticks(uint16_t* temperature_ticks,
                                                  uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0xFDU, 10000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_measure_high_precision(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_measure_high_precision_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_measure_medium_precision_ticks(uint16_t* temperature_ticks,
                                                    uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0xF6U, 1000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_measure_medium_precision(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_measure_medium_precision_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_measure_lowest_precision_ticks(uint16_t* temperature_ticks,
                                                    uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0xE0U, 1000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_measure_lowest_precision(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_measure_lowest_precision_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_activate_highest_heater_power_long_ticks(uint16_t* temperature_ticks,
                                                               uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0x39U, 1100000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_activate_highest_heater_power_long(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_activate_highest_heater_power_long_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_activate_highest_heater_power_short_ticks(uint16_t* temperature_ticks,
                                                                uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0x32U, 110000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_activate_highest_heater_power_short(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_activate_highest_heater_power_short_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_activate_medium_heater_power_long_ticks(uint16_t* temperature_ticks,
                                                              uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0x2FU, 1100000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_activate_medium_heater_power_long(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_activate_medium_heater_power_long_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_activate_medium_heater_power_short_ticks(uint16_t* temperature_ticks,
                                                               uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0x24U, 110000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_activate_medium_heater_power_short(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_activate_medium_heater_power_short_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_activate_lowest_heater_power_long_ticks(uint16_t* temperature_ticks,
                                                              uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0x1EU, 1100000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_activate_lowest_heater_power_long(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_activate_lowest_heater_power_long_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_activate_lowest_heater_power_short_ticks(uint16_t* temperature_ticks,
                                                               uint16_t* humidity_ticks) {
    return tiremo_sht40_read_measurement(0x15U, 110000U, temperature_ticks, humidity_ticks);
}

int16_t tiremo_sht40_activate_lowest_heater_power_short(int32_t* temperature, int32_t* humidity) {
    int16_t error;
    uint16_t t;
    uint16_t h;
    error = tiremo_sht40_activate_lowest_heater_power_short_ticks(&t, &h);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *temperature = tiremo_sht40_convert_ticks_to_celsius(t);
    *humidity = tiremo_sht40_convert_ticks_to_percent_rh(h);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_serial_number(uint32_t* serial_number) {
    int16_t error;
    uint8_t buffer[6];

    buffer[0] = 0x89U;
    error = tiremo_sht40_i2c_write_data(TIREMO_SHT40_I2C_ADDRESS, buffer, 1U);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    tiremo_sht40_i2c_hal_sleep_usec(1000U);
    error = tiremo_sht40_i2c_read_data_inplace(TIREMO_SHT40_I2C_ADDRESS, buffer, 4U);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    *serial_number = tiremo_sht40_bytes_to_uint32(&buffer[0]);
    return TIREMO_SHT40_NO_ERROR;
}

int16_t tiremo_sht40_soft_reset(void) {
    int16_t error;
    uint8_t cmd = 0x94U;
    error = tiremo_sht40_i2c_write_data(TIREMO_SHT40_I2C_ADDRESS, &cmd, 1U);
    if (error != TIREMO_SHT40_NO_ERROR) {
        return error;
    }
    tiremo_sht40_i2c_hal_sleep_usec(10000U);
    return TIREMO_SHT40_NO_ERROR;
}
