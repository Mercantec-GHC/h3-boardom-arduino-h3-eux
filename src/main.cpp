#include <Arduino.h>
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

CarrierUtilities _carrUtil;
BME688* _bme688;
APDS_9960* _apds9960;
ST0160* _st0160;

String _deviceId;

DeviceState _state = DISCONNECTED;

SensorData _sensorData;
unsigned long _lastSensorUpdateMs = 0;

bool _updateScreen = true;

unsigned long _lastDataTransmissionMs = 0;
unsigned long _lastWifiCheckMs = 0;

void setup() 
{
    Serial.begin(9600);

    _carrUtil.Init(USING_CARRIER_CASE);
    _carrUtil.Display_SetRotation(ROTATION_0);

    _bme688 = new BME688(_carrUtil.Get_Carrier());
    _apds9960 = new APDS_9960(_carrUtil.Get_Carrier());
    _st0160 = new ST0160(_carrUtil.Get_Carrier());

    while (!wifi_Init(_carrUtil, 3500)) 
    {
        _carrUtil.Display_Fill(ST7735_RED);
        _carrUtil.Display_PrintCentered("WIFI FAILED", 100, 2, ST7735_WHITE);
        _carrUtil.Display_PrintCentered("PRESS (04) TO TRY AGAIN", 130, 1, ST7735_WHITE);
        while (1) 
        {
            if (_carrUtil.Button_PressDown(TOUCH4)) 
            {
                break;
            }
        }
    }

    _deviceId = wifi_GetDeviceID();
    state_init(_carrUtil, _deviceId);

    _state = handleStartup();
}

void loop() 
{
    delay(100);

    if (_state == DISCONNECTED) 
    {
        _state = handleDisconnected();

        if (_state == DISCONNECTED)
        {
            return;
        }
    }

    if (_state == CONNECTED)
    {
        _state = handleConnected(_sensorData, _updateScreen);

        if (_state == DISCONNECTED)
        {
            return;
        }
    }

    if (_state == HEARTBEAT_ERROR)
    {
        _state = handleHeartbeatError();

        if (_state == CONNECTED)
        {
            _updateScreen = true;
            return;
        }
    }

    if (_state == DATA_ERROR)
    {
        _state = handleDataError(_sensorData);
        
        if (_state == CONNECTED)
        {
            _updateScreen = true;
            return;
        }
    }

    if (_state == TOKEN_ERROR)
    {
        _state = handleTokenError();

        if (_state == CONNECTED)
        {
            _updateScreen = true;
            return;
        }
    }

    if (_state == WIFI_ERROR)
    {
        _state = handleWifiError();
    }

    saveLastState(_state);

    unsigned long now = millis();

    if (now - _lastWifiCheckMs >= WIFI_CHECK_INTERVAL_MS)
    {
        if (!wifi_IsConnected())
        {
            _state = WIFI_ERROR;
        }

        _lastWifiCheckMs = now;
    }    

    if (now - _lastSensorUpdateMs >= 2500)
    {
        _lastSensorUpdateMs = now;
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

    _bme688->getTemperature(temp);
    _bme688->getHumidity(humid);
    _bme688->getPressure(pres);
    _apds9960->getLight(light);

    if (USING_ST0160)
    {
        _st0160->getMoisture(ST0160_PIN, moist);
    }
    else
    {
        moist = 0.00;
    }

    if (temp != _sensorData.Temperature || 
        humid != _sensorData.Humidity || 
        pres != _sensorData.Pressure || 
        light != _sensorData.Light || 
        moist != _sensorData.Moisture)
    {
        _updateScreen = true;
    }

    _sensorData.Temperature = temp;
    _sensorData.Humidity = humid;
    _sensorData.Pressure = pres;
    _sensorData.Light = light;
    _sensorData.Moisture = moist;
}
