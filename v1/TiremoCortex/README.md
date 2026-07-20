# TiremoCortex Example

Full Tiremo Cortex board example: sensors → optional ESP32 MQTT. **No GNSS / no SLM320.**

## Quick start

1. Open `TiremoCortex.mproj` (MCUbrew) or Eclipse workspace under `Generation/.../Example/Build/Eclipse/...`
2. Default mode: `EMPA_SENSOR_PROCESS` in `Config/app_config.h`
3. Build, flash, open debug UART
4. Press user button (PC9) to start/stop sensor cycles

## Modes (`Config/app_config.h`)

| Flag | Behavior |
|------|----------|
| `EMPA_SENSOR_PROCESS` | Button-gated UART dump of SHT40 / battery / LIS2DE12 / mic RMS |
| `EMPA_ESP32_MQTT_AWS` | WiFi MQTT publish + edge alarms (`network_config.h`, `mqtt_device_config.h`) |

Both can be enabled together.

## Layout

```
TmplUserApp/
├── prv_user_code.c          ← thin entry (Init + Run)
├── Config/                  ← app / board / network / mqtt
├── Tiremo/                  ← modular drivers (sht40, lis2de12, mic, battery, led, button, esp32, …)
├── Tiremo_Process/          ← application orchestration
│   ├── tiremo_app.c         ← init + main loop
│   ├── tiremo_app_net.c     ← ESP32 / MQTT (when flag on)
│   ├── sensor.c             ← multi-sensor facade + alarms
│   └── sensor_alarm.h
└── Mqtt_Library/            ← MQTT core + TLS certs
```

## Sensors

| Sensor | Bus | API |
|--------|-----|-----|
| SHT40 | I2C2 | `TIREMO_SHT40_App_*` |
| LIS2DE12 | I2C | `TIREMO_LIS2DE12TR_App_*` |
| Battery | ADC VCORE | `TIREMO_BAT_App_*` |
| Mic (MP23ABS1) | ADC0 + Timer1 DMA | `TIREMO_MIC_App_*` + `TIREMO_MIC_BSP_Service()` during capture |

Mic capture services DMA in the read path for ~1 s (not the old SysTick sample timer).

## Not included

- GNSS / GPS
- MeiG SLM320 4G stack (use ESP32 MQTT for cloud)
