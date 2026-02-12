#pragma once

enum DeviceState {
    DISCONNECTED,
    CONNECTED,
    HEARTBEAT_ERROR,
    DATA_ERROR,
    WIFI_ERROR,
    ERROR,
};

enum DeviceScreen {
    ALL,
    TEMPERATURE,
    HUMIDITY,
    PRESSURE,
    LIGHT,
    SETTINGS,
};

typedef struct 
{
    float Temperature;
    float Humidity;
    float Pressure;
    float Moisture;
    int Light;
} SensorData;