/**
 *******************************************************************************
 * @file        tiremo_app.h
 * @brief       TiremoCortex application process — init + main loop
 ******************************************************************************/

#ifndef TIREMO_APP_H_
#define TIREMO_APP_H_

/**
 * @brief   Board bring-up, sensors, optional ESP32/MQTT connect.
 */
void TiremoApp_Init(void);

/**
 * @brief   Forever loop: sensor cycle and/or MQTT publish.
 * @note    Does not return.
 */
void TiremoApp_Run(void);

#endif /* TIREMO_APP_H_ */
