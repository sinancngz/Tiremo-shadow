#include "abov_config.h"
#include "abov_module_config.h"
#include <stddef.h>
#include "tiremo_sht40_bsp.h"
#include "lib/tiremo_sht40_driver.h"
#include "lib/tiremo_sht40_i2c_hal.h"

static bool s_tiremoSht40Ready = false;

bool TIREMO_SHT40_BSP_Init(void) {
    if (s_tiremoSht40Ready) {
        return true;
    }

    tiremo_sht40_i2c_hal_init();
    if (tiremo_sht40_soft_reset() != 0) {
        return false;
    }

    s_tiremoSht40Ready = true;
    return true;
}

bool TIREMO_SHT40_BSP_ReadHighPrecision(int32_t* temperature_mC, int32_t* humidity_mRH) {
    if ((!s_tiremoSht40Ready) || (temperature_mC == NULL) || (humidity_mRH == NULL)) {
        return false;
    }

    return (tiremo_sht40_measure_high_precision(temperature_mC, humidity_mRH) == 0);
}

void TIREMO_SHT40_BSP_Deinit(void) {
    s_tiremoSht40Ready = false;
    tiremo_sht40_i2c_hal_free();
}
