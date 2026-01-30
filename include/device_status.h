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
    float temperature;
    float humidity;
    float pressure;
    float moisture;
    int light;
} SensorData;