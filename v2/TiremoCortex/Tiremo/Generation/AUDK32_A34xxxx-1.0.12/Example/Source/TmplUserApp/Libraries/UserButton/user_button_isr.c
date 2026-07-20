/**
 * @file    user_button_isr.c
 * @brief   PC9 user button — Port C PCU interrupt handler (HAL callback)
 */

#include "abov_config.h"
#include "abov_module_config.h"
#include "user_button.h"

void PCU_IRQHandler_PCU_ID_C(uint32_t un32Event, void *pContext)
{
    (void)pContext;
    UserButton_OnInterrupt(un32Event);
}
