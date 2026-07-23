#include "abov_config.h"
#include "abov_module_config.h"
#include "hal_i2c.h"

#include "../../../common/tiremo_systick.h"
#include "tiremo_sht40_common.h"
#include "tiremo_sht40_i2c.h"
#include "tiremo_sht40_i2c_hal.h"

int16_t tiremo_sht40_i2c_hal_select_bus(uint8_t bus_idx) {
    (void)bus_idx;
    return TIREMO_SHT40_NO_ERROR;
}

void tiremo_sht40_i2c_hal_init(void) {
}

void tiremo_sht40_i2c_hal_free(void) {
}

int8_t tiremo_sht40_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count) {
    HAL_ERR_e status;
    status = HAL_I2C_Receive(I2C_ID_2, address, data, count, true);
    return (status == HAL_ERR_OK) ? TIREMO_SHT40_NO_ERROR : TIREMO_SHT40_I2C_BUS_ERROR;
}

int8_t tiremo_sht40_i2c_hal_write(uint8_t address, const uint8_t* data, uint16_t count) {
    HAL_ERR_e status;
    status = HAL_I2C_Transmit(I2C_ID_2, address, (uint8_t*)data, count, true);
    return (status == HAL_ERR_OK) ? TIREMO_SHT40_NO_ERROR : TIREMO_SHT40_I2C_BUS_ERROR;
}

void tiremo_sht40_i2c_hal_sleep_usec(uint32_t useconds) {
    uint32_t delayMs = (useconds + 999U) / 1000U;
    if (delayMs == 0U) {
        delayMs = 1U;
    }
    TIREMO_SysTick_DelayMs(delayMs);
}
