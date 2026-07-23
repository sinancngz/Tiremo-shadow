#include "../../config/project_config.h"
#include "EMPA_Slm320.h"
#include <stdio.h>
#include <string.h>

uint8_t SLM320_ConnectBroker(void)
{
    uint8_t tryCount = 0;

    while (tryCount < SLM320_MAX_TRY_CONNECT)
    {
        slm320_state = SLM320_STATE_IDLE;

        while (slm320_state != SLM320_STATE_MQTT_PUBLISH &&
               slm320_state != SLM320_STATE_ERROR &&
               slm320_state != SLM320_STATE_DONE)
        {
            SLM320_Status_t ret = SLM320_RunStateMachine();
            if (ret == SLM320_ERROR && slm320_state != SLM320_STATE_RESET)
                break;
        }

        if (slm320_state == SLM320_STATE_MQTT_PUBLISH)
            return 0;

        tryCount++;
    }

    return 1;
}

uint8_t SLM320_ReconnectBroker(void)
{
    SLM320_PowerOff();
    SYSTICK_Wait(2000);
    slm320_state = SLM320_STATE_IDLE;
    return SLM320_ConnectBroker();
}

uint8_t SLM320_PublishSensorDataApp(const SensorData_t *pData)
{
    static char jsonBuf[256];

    Sensor_FormatJSON(pData, jsonBuf, sizeof(jsonBuf));
    return (SLM320_PublishSensorData(MQTT_TOPIC_PUB, jsonBuf) == SLM320_OK) ? 0U : 1U;
}

uint8_t SLM320_PublishAlarm(const char *jsonPayload)
{
    if (jsonPayload == NULL)
        return 1U;

    return (SLM320_PublishSensorData(MQTT_TOPIC_ALARM, jsonPayload) == SLM320_OK) ? 0U : 1U;
}
