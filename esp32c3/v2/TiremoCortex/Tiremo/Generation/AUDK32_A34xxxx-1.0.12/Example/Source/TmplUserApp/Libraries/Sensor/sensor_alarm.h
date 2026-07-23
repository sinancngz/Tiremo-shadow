#ifndef SENSOR_ALARM_H_
#define SENSOR_ALARM_H_

#include "sensor.h"
#include <stdint.h>

typedef enum
{
    SENSOR_ALARM_TEMP_HIGH = 0,
    SENSOR_ALARM_TEMP_NORMAL,
    SENSOR_ALARM_FALL,
    SENSOR_ALARM_FALL_NORMAL,
    SENSOR_ALARM_LOUD_SOUND,
    SENSOR_ALARM_SOUND_NORMAL
} SensorAlarmType_t;

#define SENSOR_ALARM_MAX_PER_CYCLE       3U

#define SENSOR_ALARM_TEMP_THRESHOLD_MC   30000   /* 30.0 C */
#define SENSOR_ALARM_FALL_REST_Z_MG      1000
#define SENSOR_ALARM_FALL_DELTA_MG       300    /* alarm when Z <= REST - DELTA */
#define SENSOR_ALARM_FALL_Z_MAX_MG       (SENSOR_ALARM_FALL_REST_Z_MG - SENSOR_ALARM_FALL_DELTA_MG)
#define SENSOR_ALARM_MIC_RMS_THRESHOLD   2000U

/**
 * @brief  Edge-triggered alarm check (MAJOR on fault, INFO on recovery).
 * @return Number of alarms to publish this cycle (0..SENSOR_ALARM_MAX_PER_CYCLE).
 */
uint8_t Sensor_PollAlarms(const SensorData_t *pData,
                          SensorAlarmType_t *pOut,
                          uint8_t maxOut);

/**
 * @brief  Format one alarm payload as JSON.
 * @return Payload length, or 0 on error.
 */
uint16_t Sensor_FormatAlarmJSON(SensorAlarmType_t alarm,
                                const SensorData_t *pData,
                                char *outBuf,
                                uint16_t bufLen);

#endif /* SENSOR_ALARM_H_ */
