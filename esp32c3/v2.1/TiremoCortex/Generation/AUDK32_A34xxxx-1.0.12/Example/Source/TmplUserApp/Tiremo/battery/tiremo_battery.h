/**
 *******************************************************************************
 * @file        tiremo_battery.h
 * @brief       Tiremo battery library — public entry point
 *
 * @details     ADC_Battery example — layered AVDD measurement library:
 *
 *              @code
 *              Tiremo/battery/
 *              ├── tiremo_battery.h        (this file — include once)
 *              ├── bsp/
 *              │   ├── tiremo_battery_board.h
 *              │   └── tiremo_battery_bsp.h/.c
 *              └── app/
 *                  └── tiremo_battery_app.h/.c
 *              @endcode
 *
 * @code
 * #include "Tiremo/battery/tiremo_battery.h"
 *
 * TIREMO_BAT_App_Init();
 * while (1) {
 *     TIREMO_BAT_App_Update();
 *     TIREMO_SysTick_DelayMs(TIREMO_BAT_APP_POLL_MS);
 * }
 * @endcode
 *
 ******************************************************************************/

#ifndef TIREMO_BATTERY_H_
#define TIREMO_BATTERY_H_

#include "bsp/tiremo_battery_bsp.h"
#include "app/tiremo_battery_app.h"

#endif /* TIREMO_BATTERY_H_ */
