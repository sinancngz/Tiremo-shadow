# MeiG SLM320 4G Driver

C driver for the **MeiG SLM320** LTE modem on the ABOV A34G43x platform. It brings up cellular data, connects to the Tiremo MQTT broker, and publishes sensor telemetry.

The driver uses a cooperative state machine (`SLM320_RunStateMachine`) called from the main loop, UART RX via interrupt, and debug logs on `UART_ID_0`.

## Related files

| File | Role |
|------|------|
| `slm320.c` / `slm320.h` | Core driver and state machine |
| `EMPA_Slm320.c` / `EMPA_Slm320.h` | Application wrapper (`SLM320_ConnectBroker`, publish helpers) |
| `../../config/mqtt_device_config.h` | Broker host, client ID, topics, TLS flag |
| `../../config/network_config.h` | APN and PDP context |
| `../cert_Lib/mqtt_certs.c` | Embedded PEM certificates (when TLS enabled) |

## Enable in the build

Set `EMPA_SLM320_4G` in `app_config.h`.

## Two MQTT modes

The SLM320 firmware exposes **two different MQTT APIs**. The driver picks the correct one at compile time via `MQTT_USE_TLS_CERTS`:

| `MQTT_USE_TLS_CERTS` | Port | Connect API | Publish API |
|----------------------|------|-------------|-------------|
| **1** (production) | 8883 | `QMTOPEN` → `QMTCONN` | `QMTPUBEX` |
| **0** (test) | 1883 | `AT+MQTTCONN` | `AT+MQTTPUB` |

**Important:** Do not use `AT+MQTTCONN` on port 8883 with TLS certificates. That command is for plain MQTT only and returns `+CME ERROR: 50` when TLS is required. For mutual TLS, upload certs with `QFUPL`, configure `QSSLCFG`, link MQTT to SSL via `QMTCFG`, then use the Quectel-style `QMT*` commands.

Similarly, `AT+QMTPUB` is not supported on this module; use **`AT+QMTPUBEX`** (returns `+CME ERROR: 58` otherwise).

## Quick start

```c
SLM320_Init(SYSTICK_GetTick);
SLM320_TickIncrement();   /* call every 1 ms from SysTick */

if (SLM320_ConnectBroker() == 0) {
    SLM320_PublishSensorDataApp(&sensorData);
}
```

`SLM320_ConnectBroker()` runs the state machine until `SLM320_STATE_MQTT_PUBLISH` is reached.

## Hardware pins

| Pin | Function |
|-----|----------|
| **PA7** | PWRKEY — hold LOW ≥ 1 s to power on |
| **PC4** | Module supply enable (CLEAR = power on) |
| **UART1** | Modem UART, 115200 8N1 |

## Logging

Only high-level milestones and errors are logged on UART0. Raw modem RX data and AT command traces are not printed.

Prefixes: `[SLM320]` for all driver messages.

## Full AT command reference

See **[SLM320_AT_Commands.md](./SLM320_AT_Commands.md)** for the complete state-machine flow, every AT command, URCs, error codes, and certificate upload sequence.
