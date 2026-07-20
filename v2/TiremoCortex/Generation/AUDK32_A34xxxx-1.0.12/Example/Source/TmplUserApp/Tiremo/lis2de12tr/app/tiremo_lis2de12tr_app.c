#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_lis2de12tr_app.h"

#include <stddef.h>

static bool s_tiremoLis2de12AppReady = false;

bool TIREMO_LIS2DE12TR_App_Init(void)
{
    if (!TIREMO_LIS2DE12TR_BSP_Init())
    {
        return false;
    }

    s_tiremoLis2de12AppReady = true;
    return true;
}

bool TIREMO_LIS2DE12TR_App_Intr_Init(void)
{
    if (!s_tiremoLis2de12AppReady)
    {
        return false;
    }

    return TIREMO_LIS2DE12TR_BSP_Intr_Init();
}

bool TIREMO_LIS2DE12TR_App_ReadAccel(int16_t* accel_x_mg, int16_t* accel_y_mg,
                                     int16_t* accel_z_mg)
{
    if ((!s_tiremoLis2de12AppReady) || (accel_x_mg == NULL) ||
        (accel_y_mg == NULL) || (accel_z_mg == NULL))
    {
        return false;
    }

    return TIREMO_LIS2DE12TR_BSP_ReadAccel(accel_x_mg, accel_y_mg, accel_z_mg);
}

bool TIREMO_LIS2DE12TR_App_ReadAccelOnce(int16_t* accel_x_mg, int16_t* accel_y_mg,
                                         int16_t* accel_z_mg)
{
    if ((!s_tiremoLis2de12AppReady) || (accel_x_mg == NULL) ||
        (accel_y_mg == NULL) || (accel_z_mg == NULL))
    {
        return false;
    }

    return TIREMO_LIS2DE12TR_BSP_ReadAccelOnce(accel_x_mg, accel_y_mg, accel_z_mg);
}

void TIREMO_LIS2DE12TR_App_Intr_Service(void)
{
    if (s_tiremoLis2de12AppReady)
    {
        TIREMO_LIS2DE12TR_BSP_Intr_Service();
    }
}

bool TIREMO_LIS2DE12TR_App_RecoverRuntime(void)
{
    if (!s_tiremoLis2de12AppReady)
    {
        return false;
    }

    return TIREMO_LIS2DE12TR_BSP_RecoverRuntime();
}

bool TIREMO_LIS2DE12TR_App_Intr_IsPending(TiremoLis2de12trIntrId_e intrId)
{
    if (!s_tiremoLis2de12AppReady)
    {
        return false;
    }

    return TIREMO_LIS2DE12TR_BSP_Intr_IsPending(intrId);
}

void TIREMO_LIS2DE12TR_App_Intr_Clear(TiremoLis2de12trIntrId_e intrId)
{
    if (s_tiremoLis2de12AppReady)
    {
        TIREMO_LIS2DE12TR_BSP_Intr_Clear(intrId);
    }
}
