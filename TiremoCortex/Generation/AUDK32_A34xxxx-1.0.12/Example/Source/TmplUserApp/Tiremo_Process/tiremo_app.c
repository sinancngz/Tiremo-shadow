/**
 *******************************************************************************
 * @file        tiremo_app.c
 * @brief       TiremoCortex application process (sensors + optional MQTT)
 *
 * @details     Keeps prv_user_code.c thin. Modes selected in Config/app_config.h:
 *              - EMPA_SENSOR_PROCESS : button-gated UART sensor dump
 *              - EMPA_ESP32_MQTT_AWS  : WiFi MQTT telemetry (+ alarms)
 *
 *              No GNSS / no SLM320 in this example (cellular stack not included).
 ******************************************************************************/

#include "tiremo_app.h"
#include "tiremo_app_net.h"
#include "sensor.h"
#include "sensor_alarm.h"

#include "../Config/project_config.h"
#include "../Tiremo/common/tiremo_systick.h"
#include "../Tiremo/DebugLibrary/debug_framework.h"
#include "../Tiremo/led/tiremo_led.h"
#include "../Tiremo/button/tiremo_button.h"

#if defined(EMPA_ESP32_MQTT_AWS)
#include "../Mqtt_Library/mqtt_core.h"
#include "../Mqtt_Library/mqtt_port.h"
extern void MqttPort_ABOV_TickIncrement(void);
#endif

/*-------------------------------------------------------------------*/
/* SysTick 1 ms extension (MQTT port + mqtt_timer)                    */
/*-------------------------------------------------------------------*/
void TIREMO_SysTick_OnTick1ms(void)
{
#if defined(EMPA_ESP32_MQTT_AWS)
    /* Always advance port tick when MQTT sources are linked. */
    MqttPort_ABOV_TickIncrement();

    if (mqtt_timer_en != 0U)
    {
        static uint16_t s_mqttTimerMs = 0U;
        s_mqttTimerMs++;
        if (s_mqttTimerMs >= 1000U)
        {
            s_mqttTimerMs = 0U;
            mqtt_timer++;
        }
    }
#endif
}

/*-------------------------------------------------------------------*/
static void TiremoApp_PrintBanner(void)
{
    DebugFramework_PutsLine("\n\r\n\r");
    DebugFramework_PutsLine("##################################");
    DebugFramework_PutsLine("#                                #");
    DebugFramework_PutsLine("#         Tiremo Cortex          #");
    DebugFramework_PutsLine("#         Example  v1.0          #");
    DebugFramework_PutsLine("#                                #");
    DebugFramework_PutsLine("##################################\n\r");
}

/*-------------------------------------------------------------------*/
void TiremoApp_Init(void)
{
    TIREMO_SysTick_Init();
    TIREMO_LED_Init();
    TIREMO_BTN_Init();
    TIREMO_BTN_App_Init();

    if (DebugFramework_Init())
    {
        TIREMO_LED_On(TIREMO_LED_1);
    }
    else
    {
        uint32_t i;
        for (i = 0U; i < 5U; i++)
        {
            TIREMO_LED_App_Blink(TIREMO_LED_1, 100U, 100U);
        }
    }

    TiremoApp_PrintBanner();

#if defined(EMPA_SENSOR_PROCESS) || defined(EMPA_ESP32_MQTT_AWS)
    Sensor_LEDTest();

    {
        uint8_t sensorFails = Sensor_TestAll();
        if (sensorFails > 0U)
        {
            DebugFramework_Printf("[WARN] %u sensor(s) failed init\n\r", sensorFails);
        }
        TIREMO_SysTick_DelayMs(1000U);
    }
#endif

#if defined(EMPA_SENSOR_PROCESS)
    DebugFramework_PutsLine("[APP] Press button to start/stop sensor cycle");
#endif
}

/*-------------------------------------------------------------------*/
void TiremoApp_Run(void)
{
#if defined(EMPA_SENSOR_PROCESS)
    uint32_t cycle = 0U;
    uint8_t cycleRunning = 0U;
#endif
#if defined(EMPA_ESP32_MQTT_AWS)
    uint8_t mqttConnected = TiremoAppNet_InitAndConnect();
    int mqtt_data_count = 1;
#endif

    while (1)
    {
#if defined(EMPA_SENSOR_PROCESS) || defined(EMPA_ESP32_MQTT_AWS)
        SensorData_t *pData = NULL;
#endif
#if defined(EMPA_ESP32_MQTT_AWS)
        SensorAlarmType_t pendingAlarms[SENSOR_ALARM_MAX_PER_CYCLE];
        uint8_t pendingAlarmCount = 0U;
#endif

#if defined(EMPA_SENSOR_PROCESS)
        if (TIREMO_BTN_App_GetEdge(TIREMO_BTN_USER) == TIREMO_BTN_EDGE_PRESSED)
        {
            cycleRunning = (cycleRunning == 0U) ? 1U : 0U;
            if (cycleRunning != 0U)
            {
                DebugFramework_PutsLine("[APP] Cycle started");
            }
            else
            {
                DebugFramework_PutsLine("[APP] Cycle stopped");
            }
        }

        if (cycleRunning == 0U)
        {
            TIREMO_SysTick_DelayMs(50U);
            continue;
        }

        DebugFramework_PutsLine("\n\r========================================");
        DebugFramework_Printf("   CYCLE #%lu\n\r", (unsigned long)cycle++);
        DebugFramework_PutsLine("========================================");
        TIREMO_LED_App_Blink(TIREMO_LED_7, 80U, 80U);
        pData = Sensor_ReadAndPrint();

        if (TIREMO_BTN_App_GetEdge(TIREMO_BTN_USER) == TIREMO_BTN_EDGE_PRESSED)
        {
            cycleRunning = 0U;
            DebugFramework_PutsLine("[APP] Cycle stopped");
            continue;
        }

#elif defined(EMPA_ESP32_MQTT_AWS)
        pData = Sensor_ReadOnly();
        pendingAlarmCount = Sensor_PollAlarms(pData, pendingAlarms, SENSOR_ALARM_MAX_PER_CYCLE);
#else
        /* No feature flag enabled — idle */
        TIREMO_SysTick_DelayMs(500U);
        continue;
#endif

#if defined(EMPA_ESP32_MQTT_AWS)
#if defined(EMPA_SENSOR_PROCESS)
        /* Sensor-process mode already filled pData; still poll alarms for MQTT. */
        pendingAlarmCount = Sensor_PollAlarms(pData, pendingAlarms, SENSOR_ALARM_MAX_PER_CYCLE);
#endif
        TiremoAppNet_PublishCycle(pData, &mqttConnected, &mqtt_data_count);
        TiremoAppNet_PublishAlarms(pData, pendingAlarms, pendingAlarmCount, mqttConnected);
#endif

        TIREMO_SysTick_DelayMs(APP_PUBLISH_INTERVAL_MS);
    }
}
