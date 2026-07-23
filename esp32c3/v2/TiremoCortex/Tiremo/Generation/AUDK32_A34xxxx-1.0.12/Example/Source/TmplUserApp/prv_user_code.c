/**
 *******************************************************************************
 * @file        prv_user_code.c
 * @author      ABOV R&D Division
 * @brief       Dummy User Application Main
 *
 * Copyright 2024 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "hal_pcu.h"
#include "hal_i2c.h"
#include "hal_timer1.h"
#include "Libraries/DebugLibrary/debug_framework.h"
#include "Libraries/ESP32_AT_Test/ESP32_AT_Test.h"
#include "Libraries/MQTT_Library/EMPA_MqttAws.h"
#include "Libraries/Sensor/sensor.h"
#include "Libraries/Sensor/sensor_alarm.h"
#include "config/project_config.h"
#include "Libraries/UserButton/user_button.h"
#include "Libraries/MEIG_SLM3XX/EMPA_Slm320.h"
#include "Libraries/cert_Lib/mqtt_cert_provision.h"
#include "Libraries/cert_Lib/mqtt_certs.h"


extern void MqttPort_ABOV_Init(void);
extern void MqttPort_ABOV_TickIncrement(void);
extern void MP23ABS1_TimerHandler(void);

extern uint32_t SystemCoreClock;
static volatile uint32_t s_un32SysTimerVal=0;

void LED_ON(PCU_ID_e port, PCU_PIN_ID_e pin);
void LED_OFF(PCU_ID_e port, PCU_PIN_ID_e pin);

/**********************************************************************
 * @brief		ARM System Timer Interrupt Handler.
 * @param[in]	None
 * @return	    None
 * @note        Also calls MP23ABS1_TimerHandler for microphone sampling
 **********************************************************************/
void SysTick_Handler (void)
{
    static uint32_t s_un32TickCounter = 0;

    if (s_un32SysTimerVal)
    {
        s_un32SysTimerVal--;
    }

    /* MQTT port tick (1ms) - needed for UART receive timeout */
    MqttPort_ABOV_TickIncrement();
    UserButton_Tick1ms();

    /* mqtt_timer: 1 tick per second (thresholds in mqtt_core.c are in seconds) */
    if (mqtt_timer_en) {
        static uint16_t s_mqttTimerMs = 0;
        s_mqttTimerMs++;
        if (s_mqttTimerMs >= 1000U) {
            s_mqttTimerMs = 0U;
            mqtt_timer++;
        }
    }

#if defined(EMPA_SLM320_4G)
    SLM320_TickIncrement();
#endif

    /* Call MP23ABS1 timer handler every 16 ticks (for 1kHz SysTick, this gives ~62.5us per sample) */
    s_un32TickCounter++;
    if (s_un32TickCounter >= 16)
    {
        s_un32TickCounter = 0;
        MP23ABS1_TimerHandler();
    }
}

/**********************************************************************
 * @brief		Waiting by time(ms)
 * @param[in]	un32TimeMS : Milisecond time to wait.
 * @return	    None
 **********************************************************************/
void SYSTICK_Wait (uint32_t un32TimeMS)
{
    s_un32SysTimerVal = un32TimeMS;
    while (s_un32SysTimerVal);
}

/**********************************************************************
 * @brief       Turn ON specific LED
 * @param[in]   LED_PIN : LED pin ID
 * @return      None
 **********************************************************************/
void LED_ON(PCU_ID_e port, PCU_PIN_ID_e pin)
{
    HAL_PCU_SetOutputBit(port, pin, PCU_OUTPUT_BIT_CLEAR);
}

/**********************************************************************
 * @brief       Turn OFF specific LED
 * @param[in]   LED_PIN : LED pin ID
 * @return      None
 **********************************************************************/
void LED_OFF(PCU_ID_e port, PCU_PIN_ID_e pin)
{
    HAL_PCU_SetOutputBit(port, pin, PCU_OUTPUT_BIT_SET);
}

/**********************************************************************
 * @brief       Blink specific LED
 * @param[in]   LED_PIN : LED pin ID
 * @param[in]   times : Number of blinks
 * @return      None
 **********************************************************************/
void LED_Blink(PCU_ID_e port, PCU_PIN_ID_e pin, uint32_t times)
{
    for(uint32_t i = 0; i < times; i++)
    {
        LED_ON(port, pin);
        SYSTICK_Wait(100);
        LED_OFF(port, pin);
        SYSTICK_Wait(100);
    }
}

/**********************************************************************
 * @brief       Configure GPIO Alternate Functions
 * @param[in]   None
 * @return      None
 **********************************************************************/
void GPIO_Config_Alt()
{
    /* Configure all Test LEDs as GPIO output (ALT_0) */
    HAL_PCU_SetAltMode(BOARD_LED1_PORT, BOARD_LED1_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED2_PORT, BOARD_LED2_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED3_PORT, BOARD_LED3_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED4_PORT, BOARD_LED4_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED5_PORT, BOARD_LED5_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED6_PORT, BOARD_LED6_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED7_PORT, BOARD_LED7_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED8_PORT, BOARD_LED8_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED9_PORT, BOARD_LED9_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_LED10_PORT, BOARD_LED10_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_USER_BTN_PORT, BOARD_USER_BTN_PIN, PCU_ALT_0);
#if defined(EMPA_SLM320_4G)
    HAL_PCU_SetAltMode(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(SLM320_PWR_PORT, SLM320_PWR_PIN, PCU_ALT_0);
    HAL_PCU_SetAltMode(BOARD_MODEM_UART_PORT, BOARD_MODEM_UART_RX_PIN, BOARD_MODEM_UART_ALT);
    HAL_PCU_SetAltMode(BOARD_MODEM_UART_PORT, BOARD_MODEM_UART_TX_PIN, BOARD_MODEM_UART_ALT);
#endif

    /* Configure I2C pins */
    HAL_PCU_SetAltMode(BOARD_I2C_SCL_PORT, BOARD_I2C_SCL_PIN, BOARD_I2C_ALT_FUNCTION);
    HAL_PCU_SetAltMode(BOARD_I2C_SDA_PORT, BOARD_I2C_SDA_PIN, BOARD_I2C_ALT_FUNCTION);

    HAL_PCU_SetAltMode(BOARD_ESP32_UART_PORT, BOARD_ESP32_UART_RX_PIN, BOARD_ESP32_UART_ALT);
    HAL_PCU_SetAltMode(BOARD_ESP32_UART_PORT, BOARD_ESP32_UART_TX_PIN, BOARD_ESP32_UART_ALT);
    HAL_PCU_SetAltMode(BOARD_ESP32_PWR_PORT, BOARD_ESP32_PWR_PIN, PCU_ALT_0);
}

/**********************************************************************
 * @brief       User Code Here
 * @param[in]   None
 * @return      None
 **********************************************************************/
void PRV_USER_Code(void)
{
    SysTick_Config(SystemCoreClock / APP_SYSTICK_1MS_DIV);
    GPIO_Config_Alt();
    UserButton_Init();

    /* 1. Debug Framework Init */
    if (DebugFramework_Init())
    {
        LED_ON(BOARD_LED1_PORT, BOARD_LED1_PIN);
    }
    else
    {
        LED_Blink(BOARD_LED1_PORT, BOARD_LED1_PIN, 5);
    }

    DebugFramework_PutsLine("\n\r\n\r");
    DebugFramework_PutsLine("##################################");
	DebugFramework_PutsLine("#                                #");
    DebugFramework_PutsLine("#         Tiremo Cortex          #");
    DebugFramework_PutsLine("#              v2.0              #");
	DebugFramework_PutsLine("#                                #");
    DebugFramework_PutsLine("##################################\n\r");


    /* 2. LED Hardware Test */
#if defined(EMPA_SENSOR_PROCESS) || defined(EMPA_ESP32_MQTT_AWS) || defined(EMPA_SLM320_4G)
    Sensor_LEDTest();
#endif

    /* 3. Sensor init */
#if defined(EMPA_SENSOR_PROCESS) || defined(EMPA_ESP32_MQTT_AWS) || defined(EMPA_SLM320_4G)
    {
        uint8_t sensorFails = Sensor_TestAll();
        if (sensorFails > 0U)
        {
            DebugFramework_Printf("[WARN] %u sensor(s) failed init\n\r", sensorFails);
        }
        SYSTICK_Wait(1000);
    }
#endif

#if defined(EMPA_SENSOR_PROCESS)
    DebugFramework_PutsLine("[APP] Press button to start/stop sensor cycle");
#endif

    /* 4. SLM320 4G Module Init */
#if defined(EMPA_SLM320_4G)
    DebugFramework_PutsLine("\n\r========================================");
    DebugFramework_PutsLine("       SLM320 4G LTE MODULE INIT");
    DebugFramework_PutsLine("========================================\n\r");

    SLM320_Init(NULL);
    DebugFramework_PutsLine("[SLM320] Driver init OK");
#endif

    /* 4. ESP32 AT Test */
#if defined(EMPA_ESP32_MQTT_AWS)
    DebugFramework_PutsLine("\n\r========================================");
    DebugFramework_PutsLine("       ESP32 & MQTT CONNECTIVITY TEST       ");
    DebugFramework_PutsLine("========================================\n\r");

    DebugFramework_PutsLine("[TEST] ESP32-AT WiFi Module...");
    if (ESP32_AT_Test_WithRecovery() == 0)
    {
        DebugFramework_PutsLine("[PASS] ESP32 OK");
    }
    else
    {
        DebugFramework_PutsLine("[FAIL] ESP32 NOT responding");
    }

    MqttPort_ABOV_Init();
#endif

    /* 4.5 TLS certificate provisioning (shared by ESP32 and/or SLM320) */
#if (defined(EMPA_ESP32_MQTT_AWS) || defined(EMPA_SLM320_4G)) && MQTT_USE_TLS_CERTS
#if defined(EMPA_ESP32_MQTT_AWS)
    extern char mqttPacketBuffer[];
#endif
    if (MqttCerts_HasEmbedded() != 0U)
    {
#if defined(EMPA_ESP32_MQTT_AWS)
        if (MqttCertProv_Run(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
#else
        if (MqttCertProv_Run(NULL, 0U) != 0)
#endif
            DebugFramework_PutsLine("[CERT] Provisioning failed — will try MQTT anyway");
    }
    else
    {
        DebugFramework_PutsLine("[CERT] No certificates on ABOV");
        MqttCerts_LogFlashIfPresent();
    }
#endif

    /* 5. MQTT Connect */
#if defined(EMPA_ESP32_MQTT_AWS)
    DebugFramework_PutsLine("\n\r[MQTT] Connecting to broker...");
    uint8_t mqttConnected = (MQTT_ConnectBroker() == 0) ? 1 : 0;
    if (mqttConnected)
        DebugFramework_PutsLine("[MQTT] Ready to publish sensor data\n\r");
    else
        DebugFramework_PutsLine("[MQTT] Will retry each cycle\n\r");
#endif

    /* 5. SLM320 4G MQTT Connect */
#if defined(EMPA_SLM320_4G)
    DebugFramework_PutsLine("\n\r[SLM320] Starting 4G LTE connection...");
    uint8_t slm320Connected = (SLM320_ConnectBroker() == 0) ? 1 : 0;
    if (slm320Connected)
        DebugFramework_PutsLine("[SLM320] MQTT broker connected!");
    else
        DebugFramework_PutsLine("[SLM320] MQTT connect failed, will retry next cycle");
    int slm320_data_count = 1;
#endif

    /* 6. Main Loop */
    uint32_t cycle = 0U;
#if defined(EMPA_SENSOR_PROCESS)
    uint8_t cycleRunning = 0U;
#endif
    int mqtt_data_count = 1;
    while (1)
    {
        SensorAlarmType_t pendingAlarms[SENSOR_ALARM_MAX_PER_CYCLE];
        uint8_t pendingAlarmCount = 0U;

#if defined(EMPA_SENSOR_PROCESS)
        if (UserButton_ConsumeShortPress() != 0U)
        {
            cycleRunning = (cycleRunning == 0U) ? 1U : 0U;
            if (cycleRunning != 0U)
                DebugFramework_PutsLine("[APP] Cycle started");
            else
                DebugFramework_PutsLine("[APP] Cycle stopped");
        }

        if (cycleRunning == 0U)
        {
            SYSTICK_Wait(50U);
            continue;
        }

        DebugFramework_PutsLine("\n\r========================================");
        DebugFramework_Printf("   CYCLE #%lu\n\r", (unsigned long)cycle++);
        DebugFramework_PutsLine("========================================");
        LED_Blink(BOARD_LED7_PORT, BOARD_LED7_PIN, 1U);
        SensorData_t *pData = Sensor_ReadAndPrint();
        (void)pData;

        if (UserButton_ConsumeShortPress() != 0U)
        {
            cycleRunning = 0U;
            DebugFramework_PutsLine("[APP] Cycle stopped");
            continue;
        }
#elif defined(EMPA_ESP32_MQTT_AWS) || defined(EMPA_SLM320_4G)
        SensorData_t *pData = Sensor_ReadOnly();
        pendingAlarmCount = Sensor_PollAlarms(pData, pendingAlarms, SENSOR_ALARM_MAX_PER_CYCLE);
#endif

#if defined(EMPA_ESP32_MQTT_AWS)
        if (mqttConnected)
        {
            DebugFramework_PutsLine("[MQTT] Data sending...");
            MQTT_PublishSensorData(pData);
            DebugFramework_Printf("[MQTT] Data sent! #%d\n\r", mqtt_data_count++);
        }
        else
        {
            /* Connection lost — turn off LED6 and LED7 */
            LED_OFF(BOARD_LED_MQTT_TX_PORT, BOARD_LED_MQTT_TX_PIN);
            LED_OFF(BOARD_LED_WIFI_STATUS_PORT, BOARD_LED_WIFI_STATUS_PIN);
#if defined(EMPA_SENSOR_PROCESS)
            DebugFramework_PutsLine("[MQTT] Not connected, retrying...");
#endif
            mqttConnected = (MQTT_ConnectBroker() == 0) ? 1 : 0;
            if (mqttConnected)
            {
                MQTT_PublishSensorData(pData);
#if defined(EMPA_SENSOR_PROCESS)
                DebugFramework_PutsLine("[MQTT] Reconnected & data sent!");
#endif
            }
        }
#endif

#if defined(EMPA_SLM320_4G)
        if (slm320Connected)
        {
            DebugFramework_PutsLine("[SLM320] Sending sensor data...");
            if (SLM320_PublishSensorDataApp(pData) == 0)
            {
                DebugFramework_Printf("[SLM320] Data sent! #%d\n\r", slm320_data_count++);
            }
            else
            {
                DebugFramework_PutsLine("[SLM320] Data send failed!");
                slm320Connected = 0;
            }
        }
        else
        {
            DebugFramework_PutsLine("[SLM320] Not connected, reconnecting...");
            slm320Connected = (SLM320_ReconnectBroker() == 0) ? 1 : 0;
            if (slm320Connected)
            {
                DebugFramework_PutsLine("[SLM320] Reconnected successfully!");
                SLM320_PublishSensorData(MQTT_TOPIC_PUB, "reconnect_test");
            }
        }
#endif

#if defined(EMPA_ESP32_MQTT_AWS) || defined(EMPA_SLM320_4G)
        if (pendingAlarmCount > 0U)
        {
            char alarmBuf[160];
            uint8_t ai;

            for (ai = 0U; ai < pendingAlarmCount; ai++)
            {
                if (Sensor_FormatAlarmJSON(pendingAlarms[ai], pData,
                                           alarmBuf, sizeof(alarmBuf)) == 0U)
                    continue;

#if defined(EMPA_ESP32_MQTT_AWS)
                if (mqttConnected)
                    MQTT_PublishAlarm(alarmBuf);
#endif

#if defined(EMPA_SLM320_4G)
                if (slm320Connected)
                    (void)SLM320_PublishAlarm(alarmBuf);
#endif
            }
        }
#endif

    SYSTICK_Wait(APP_PUBLISH_INTERVAL_MS);
    }
}
