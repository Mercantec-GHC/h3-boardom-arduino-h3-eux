#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include <carrier_utilities.h>
#include <config.h>
#include <wifi_handle.h>
#include <device_status.h>

#include <data_transmission.h>
#include <state_logic.h>

#include <bme688.h>
#include <apds-9960.h>
#include <st0160.h>

void updateSensorData();

MKRIoTCarrier carrier;
CarrierUtilities carrUtil(carrier);
BME688 bme688(carrier);
APDS_9960 apds9960(carrier);
ST0160 st0160(carrier);

String deviceId;

DeviceState state = DISCONNECTED;

SensorData sensorData;
unsigned long lastSensorUpdateMs = 0;

bool updateScreen = true;

bool hasRun = false;

unsigned long lastDataTransmissionMs = 0;
unsigned long lastWifiCheckMs = 0;

void setup() 
{
    Serial.begin(9600);

    USING_CARRIER_CASE ? carrier.withCase() : carrier.noCase();

    carrier.begin();

    carrUtil.Display_SetRotation(ROTATION_0);

    while (!wifi_Init(carrUtil, 3500)) 
    {
        carrUtil.Display_Fill(ST7735_RED);
        carrUtil.Display_PrintCentered("WIFI FAILED", 100, 2, ST7735_WHITE);
        carrUtil.Display_PrintCentered("PRESS (04) TO TRY AGAIN", 130, 1, ST7735_WHITE);
        while (1) 
        {
            if (carrUtil.Button_PressDown(TOUCH4)) 
            {
                break;
            }
        }
    }

    deviceId = wifi_GetDeviceID();
    state_init(carrUtil, deviceId);
}

void loop() 
{
    delay(100);

    unsigned long now = millis();

    if (!hasRun) 
    {
        state = handleInitialHeartbeat();
        hasRun = true;
    }

    if (state == DISCONNECTED) 
    {
        state = handleDisconnected(now);

        if (state == DISCONNECTED)
        {
            return;
        }
    }

    if (state == CONNECTED)
    {
        state = handleConnected(sensorData, updateScreen, now);
    }

    if (state == HEARTBEAT_ERROR)
    {
        state = handleHeartbeatError(now);
    }

    if (state == DATA_ERROR)
    {
        state = handleDataError(sensorData);
    }

    if (state == WIFI_ERROR)
    {
        state = handleWifiError();
    }

    saveLastState(state);

    if (now - lastWifiCheckMs >= WIFI_CHECK_INTERVAL_MS)
    {
        if (!wifi_IsConnected())
        {
            state = WIFI_ERROR;
        }

        lastWifiCheckMs = now;
    }    

    if (now - lastSensorUpdateMs >= 2500)
    {
        lastSensorUpdateMs = now;
        updateSensorData();
    }
}

void updateSensorData()
{
    float temp;
    float humid;
    float pres;
    float moist;
    int light;

    bme688.getTemperature(temp);
    bme688.getHumidity(humid);
    bme688.getPressure(pres);
    apds9960.getLight(light);

    if (USING_ST0160)
    {
        st0160.getMoisture(ST0160_PIN, moist);
    }
    else
    {
        moist = 0.00;
    }

    if (temp != sensorData.temperature || 
        humid != sensorData.humidity || 
        pres != sensorData.pressure || 
        light != sensorData.light || 
        moist != sensorData.moisture)
    {
        updateScreen = true;
    }

    sensorData.temperature = temp;
    sensorData.humidity = humid;
    sensorData.pressure = pres;
    sensorData.light = light;
    sensorData.moisture = moist;
}
