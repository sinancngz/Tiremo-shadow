#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_sht40_app.h"

#include "../bsp/tiremo_sht40_bsp.h"
#include <stddef.h>

static bool s_tiremoSht40AppReady = false;

bool TIREMO_SHT40_App_Init(void) {
    if (!TIREMO_SHT40_BSP_Init()) {
        return false;
    }

    s_tiremoSht40AppReady = true;
    return true;
}

bool TIREMO_SHT40_App_Read(int32_t* temperature_mC, int32_t* humidity_mRH) {
    if ((!s_tiremoSht40AppReady) || (temperature_mC == NULL) || (humidity_mRH == NULL)) {
        return false;
    }

    return TIREMO_SHT40_BSP_ReadHighPrecision(temperature_mC, humidity_mRH);
}
