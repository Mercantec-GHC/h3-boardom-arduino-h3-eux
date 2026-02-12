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

CarrierUtilities carrUtil;
BME688* bme688;
APDS_9960* apds9960;
ST0160* st0160;

String deviceId;

DeviceState state = DISCONNECTED;

SensorData sensorData;
unsigned long lastSensorUpdateMs = 0;

bool updateScreen = true;

unsigned long lastDataTransmissionMs = 0;
unsigned long lastWifiCheckMs = 0;

void setup() 
{
    Serial.begin(9600);

    carrUtil.Init(USING_CARRIER_CASE);

    bme688 = new BME688(carrUtil.Get_Carrier());
    apds9960 = new APDS_9960(carrUtil.Get_Carrier());
    st0160 = new ST0160(carrUtil.Get_Carrier());

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
    state = handleInitialHeartbeat();
}

void loop() 
{
    delay(100);

    unsigned long now = millis();

    if (state == DISCONNECTED) 
    {
        state = handleDisconnected();

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

    bme688->getTemperature(temp);
    bme688->getHumidity(humid);
    bme688->getPressure(pres);
    apds9960->getLight(light);

    if (USING_ST0160)
    {
        st0160->getMoisture(ST0160_PIN, moist);
    }
    else
    {
        moist = 0.00;
    }

    if (temp != sensorData.Temperature || 
        humid != sensorData.Humidity || 
        pres != sensorData.Pressure || 
        light != sensorData.Light || 
        moist != sensorData.Moisture)
    {
        updateScreen = true;
    }

    sensorData.Temperature = temp;
    sensorData.Humidity = humid;
    sensorData.Pressure = pres;
    sensorData.Light = light;
    sensorData.Moisture = moist;
}
