/**
 *******************************************************************************
 * @file        prv_user_code.c
 * @brief       TiremoCortex user entry — delegates to Tiremo_Process
 *
 * Copyright 2024 ABOV Semiconductor Co.,Ltd. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

#include "Tiremo_Process/tiremo_app.h"

/**********************************************************************
 * @brief       User application entry (called from main after HAL init)
 **********************************************************************/
void PRV_USER_Code(void)
{
    TiremoApp_Init();
    TiremoApp_Run();
}
