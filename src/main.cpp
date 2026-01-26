#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include <carrier_utilities.h>
#include <config.h>
#include <wifi_handle.h>

#include <data-transmission.h>

#include <bme688.h>
#include <apds-9960.h>
#include <st0160.h>

void updateSensorData();
void writeArrows();

enum DeviceState {
    DISCONNECTED,
    CONNECTED,
    HEARTBEAT_ERROR,
    ERROR,
};

enum DeviceScreen {
    ALL,
    TEMPERATURE,
    HUMIDITY,
    PRESSURE,
    LIGHT,
};

typedef struct 
{
    float temperature;
    float humidity;
    float pressure;
    float moisture;
    int light;
} SensorData;

MKRIoTCarrier carrier;
CarrierUtilities carrUtil(carrier);
BME688 bme688(carrier);
APDS_9960 apds9960(carrier);
ST0160 st0160(carrier);

DataTransmitter dataTransmit;

String deviceId;

DeviceState state = DISCONNECTED;
DeviceState lastState = ERROR;
DeviceScreen screen = ALL;

SensorData sensorData;
unsigned long lastSensorUpdateMs = 0;

bool updateScreen = true;

bool hasRun = false;

uint32_t heartbeatIntervalMs = 100000; // Every 100 seconds by default
unsigned long lastHeartbeatMs = 0;

unsigned long lastDataTransmissionMs = 0;

void setup() 
{
    Serial.begin(9600);

    USING_CARRIER_CASE ? carrier.withCase() : carrier.noCase();

    randomSeed(micros());
    heartbeatIntervalMs = random(60000, 180000); // Random interval between 1 and 2 minutes

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
}

void loop() 
{
    delay(100);

    unsigned long now = millis();

    if (!hasRun) 
    {
        if (dataTransmit.sendHeartbeat(deviceId)) 
        {
            state = CONNECTED;
        } 
        else 
        {
            Serial.print("Initial Heartbeat failed - setting state to DISCONNECTED");
            state = DISCONNECTED;
        }

        hasRun = true;
    }

    if (state == DISCONNECTED) 
    {
        if (lastState != DISCONNECTED)
        {
            carrUtil.Display_Fill(COLOR_BLUE);
            carrUtil.Display_PrintCentered(deviceId, 70, 1, COLOR_WHITE);
            carrUtil.Display_PrintCentered("DISCONNECTED", 110, 2, COLOR_WHITE);
            carrUtil.Display_PrintCentered("PRESS (04) TO CONNECT TO:", 140, 1, COLOR_WHITE);
            carrUtil.Display_PrintCentered(String(SERVER_IP) + ":" + String(DASHBOARD_PORT), 155, 1, COLOR_WHITE);
        }

        if (carrUtil.Button_PressDown(TOUCH4))
        {
            carrUtil.Display_Fill(COLOR_BLUE);
            carrUtil.Display_PrintCentered("CONNECTING...", 110, 2, COLOR_WHITE);
        }
    }

    if (state == CONNECTED)
    {
        if (lastState != CONNECTED)
        {
            carrUtil.Display_Fill(COLOR_GREEN);
            carrUtil.Display_PrintCentered(deviceId, 60, 2, COLOR_WHITE);
            carrUtil.Display_PrintCentered("CONNECTED", 100, 2, COLOR_WHITE);
        }

        if (updateScreen)
        {
            carrUtil.Display_Fill(COLOR_DARK_GREEN);
            carrUtil.Display_PrintCentered(String(deviceId), 20, 1, COLOR_WHITE);

            if (screen == ALL)
            {
                carrUtil.Display_PrintCentered("ALL DATA", 50, 2, COLOR_WHITE);
                carrUtil.Display_PrintCentered("TEMP | HUM | PRES | LIGHT", 85, 1, COLOR_WHITE);
                carrUtil.Display_PrintCentered(String(sensorData.temperature, 2) + " C | " + String(sensorData.humidity, 2) + "% | " + String(sensorData.pressure, 2) + " hPa | " + String(sensorData.light), 110, 1, COLOR_WHITE);
                writeArrows();
            }
            else if (screen == TEMPERATURE)
            {
                carrUtil.Display_PrintCentered("TEMPERATURE", 70, 2, COLOR_WHITE);
                carrUtil.Display_PrintCentered(String(sensorData.temperature, 2) + " C", 110, 3, COLOR_WHITE);
                writeArrows();
            }
            else if (screen == HUMIDITY)
            {
                carrUtil.Display_PrintCentered("HUMIDITY", 70, 2, COLOR_WHITE);
                carrUtil.Display_PrintCentered(String(sensorData.humidity, 2) + " %", 110, 3, COLOR_WHITE);
                writeArrows();
            }
            else if (screen == PRESSURE)
            {
                carrUtil.Display_PrintCentered("PRESSURE", 70, 2, COLOR_WHITE);
                carrUtil.Display_PrintCentered(String(sensorData.pressure, 2) + " hPa", 110, 3, COLOR_WHITE);
                writeArrows();
            }
            else if (screen == LIGHT)
            {
                carrUtil.Display_PrintCentered("LIGHT", 70, 2, COLOR_WHITE);
                carrUtil.Display_PrintCentered(String(sensorData.light), 110, 3, COLOR_WHITE);
                writeArrows();
            }

            updateScreen = false;
        }

        if (carrUtil.Button_PressDown(TOUCH4))
        {
            screen = static_cast<DeviceScreen>((screen + 1) % 5);
            updateScreen = true;
        }
        else if (carrUtil.Button_PressDown(TOUCH0))
        {
            screen = static_cast<DeviceScreen>((screen - 1 + 5) % 5);
            updateScreen = true;
        }


        if (now - lastHeartbeatMs >= heartbeatIntervalMs)
        {
            Serial.print("Sending Heartbeat...");
            if (dataTransmit.sendHeartbeat(deviceId)) 
            {
                lastHeartbeatMs = now;
            } 
            else 
            {
                Serial.print("Heartbeat failed - setting state to HEARTBEAT_ERROR");
                state = HEARTBEAT_ERROR;
            }
        }

    }

    if (state == HEARTBEAT_ERROR)
    {
        if (lastState != HEARTBEAT_ERROR)
        {
            carrUtil.Display_Fill(ST7735_RED);
            carrUtil.Display_PrintCentered(deviceId, 60, 2, ST7735_WHITE);
            carrUtil.Display_PrintCentered("HEARTBEAT ERROR", 100, 2, ST7735_WHITE);
            carrUtil.Display_PrintCentered("PRESS (02) TO RETRY", 130, 1, ST7735_WHITE);
        }

        if (carrUtil.Button_PressDown(TOUCH2))
        {
            carrUtil.Display_Fill(ST7735_BLUE);
            carrUtil.Display_PrintCentered("RETRYING HEARTBEAT", 105, 2, ST7735_WHITE);

            if (dataTransmit.sendHeartbeat(deviceId)) 
            {
                state = CONNECTED;
                lastHeartbeatMs = now;
            } 
            else 
            {
                Serial.print("Heartbeat retry failed - staying in HEARTBEAT_ERROR");
                state = HEARTBEAT_ERROR;
            }
        }
    }

    lastState = state;

    if (now - lastSensorUpdateMs >= 2500)
    {
        lastSensorUpdateMs = now;
        updateSensorData();
    }

    if (now - lastDataTransmissionMs >= DATA_INTERVAL_MS)
    {
        lastDataTransmissionMs = now;

        Serial.print("Transmitting Data...");

        if (dataTransmit.sendData(deviceId, sensorData.temperature, sensorData.humidity, sensorData.pressure, sensorData.light, sensorData.moisture)) 
        {
            Serial.println("Data Transmission Successful");
        } 
        else 
        {
            Serial.println("Data Transmission Failed");
        }
    }
}

void writeArrows()
{
    carrUtil.Display_Print("<-", 30, 160, 2, COLOR_WHITE);
    carrUtil.Display_Print("->", 190, 160, 2, COLOR_WHITE);
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
    st0160.getMoisture(A0, moist);

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
