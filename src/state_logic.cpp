#include <state_logic.h>
#include <config.h>
#include <data_transmission.h>

DataTransmitter _dataTransmit;

uint32_t heartbeatIntervalMs = 100000;
unsigned long lastHeartbeatMs = 0;

DeviceState lastState = ERROR;

DeviceScreen screen = ALL;
DeviceScreen lastScreen = LIGHT;

unsigned long lastdataTransmissionMs = 0;

uint16_t timeUpdateMs = 1000;
unsigned long lastTimeUpdateMs = 0;

CarrierUtilities* _carrUtil;
String _devId;

void state_init(CarrierUtilities& carrUtil, String devId)
{
    _carrUtil = &carrUtil;
    _devId = devId;

    randomSeed(analogRead(A6) ^ micros() ^ millis());
    heartbeatIntervalMs = random(60000, 180000); // Random interval between 1 and 2 minutes
}

void writeArrows()
{
    _carrUtil->Display_Print("<-", 30, 160, 2, COLOR_WHITE);
    _carrUtil->Display_Print("->", 190, 160, 2, COLOR_WHITE);
}

void writeRemainingTime()
{
    uint8_t secondsRemaining = (DATA_INTERVAL_MS - (millis() - lastdataTransmissionMs)) / 1000;
        
    _carrUtil->Display_FillRect(0, 140, 240, 10, COLOR_DARK_GREEN);
    _carrUtil->Display_PrintCentered("SENDING DATA IN: " + String(secondsRemaining) + " SECONDS", 140, 1, COLOR_WHITE);
}

void saveLastState(DeviceState newState)
{
    lastState = newState;
}

DeviceState handleInitialHeartbeat()
{
    if (_dataTransmit.sendHeartbeat(_devId))
    {
        return CONNECTED;
    }
    
    return DISCONNECTED;
}

DeviceState handleDisconnected(unsigned long now)
{
    if (lastState != DISCONNECTED)
    {
        _carrUtil->Display_Fill(COLOR_BLUE);
        _carrUtil->Display_PrintCentered(_devId, 40, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("DISCONNECTED", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS (04) TO CONNECT TO:", 150, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered(String(SERVER_IP) + ":" + String(DASHBOARD_PORT), 165, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH4))
    {
        _carrUtil->Display_Fill(COLOR_BLUE);
        _carrUtil->Display_PrintCentered("CONNECTING", 110, 2, COLOR_WHITE);

        if (_dataTransmit.connectDashboard(_devId))
        {
            unsigned long startMs = millis();
            unsigned long lastHbAttemptMs = 0;
            uint16_t timeoutMs = 20000;

            _carrUtil->Display_SetCursor(30, 130);

            while (millis() - startMs < timeoutMs)
            {
                delay(timeoutMs / 40);
                _carrUtil->Display_PrintDefault(".", 1, COLOR_WHITE);
                
                if (millis() - lastHbAttemptMs >= 2500)
                {
                        if (_dataTransmit.sendHeartbeat(_devId))
                        {
                            return CONNECTED;
                        }
                }

                lastHbAttemptMs = startMs;
            }
            
            lastState = HEARTBEAT_ERROR; // Just something other than DISCONNECTED
            return DISCONNECTED;
        }
        
        lastState = HEARTBEAT_ERROR; // Just something other than DISCONNECTED
        return DISCONNECTED;
    }

    saveLastState(DISCONNECTED);
    return DISCONNECTED;
}

DeviceState handleConnected(SensorData sensorData, bool& updateScreen, unsigned long now)
{
    if (lastState != CONNECTED)
    {
        _carrUtil->Display_Fill(COLOR_GREEN);
        _carrUtil->Display_PrintCentered(_devId, 60, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("CONNECTED", 100, 2, COLOR_WHITE);
    }

    if (updateScreen)
    {
        if (screen == SETTINGS)
        {
            _carrUtil->Display_Fill(COLOR_BLACK);
        }
        else
        {   
            _carrUtil->Display_Fill(COLOR_DARK_GREEN);
            _carrUtil->Display_PrintCentered("SETTINGS", 20, 1, COLOR_WHITE);
        }

        if (screen == ALL)
        {
            _carrUtil->Display_PrintCentered("ALL DATA", 50, 2, COLOR_WHITE);
            _carrUtil->Display_PrintCentered("TEMP | HUM | PRES | LIGHT", 85, 1, COLOR_WHITE);
            _carrUtil->Display_PrintCentered(String(sensorData.temperature, 2) + " C | " + String(sensorData.humidity, 2) + "% | " + String(sensorData.pressure, 2) + " hPa | " + String(sensorData.light), 110, 1, COLOR_WHITE);
            writeArrows();
            writeRemainingTime();
        }
        else if (screen == TEMPERATURE)
        {
            _carrUtil->Display_PrintCentered("TEMPERATURE", 70, 2, COLOR_WHITE);
            _carrUtil->Display_PrintCentered(String(sensorData.temperature, 2) + " C", 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == HUMIDITY)
        {
            _carrUtil->Display_PrintCentered("HUMIDITY", 70, 2, COLOR_WHITE);
            _carrUtil->Display_PrintCentered(String(sensorData.humidity, 2) + " %", 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == PRESSURE)
        {
            _carrUtil->Display_PrintCentered("PRESSURE", 70, 2, COLOR_WHITE);
            _carrUtil->Display_PrintCentered(String(sensorData.pressure, 2) + " hPa", 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == LIGHT)
        {
            _carrUtil->Display_PrintCentered("LIGHT", 70, 2, COLOR_WHITE);
            _carrUtil->Display_PrintCentered(String(sensorData.light), 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == SETTINGS)
        {
            _carrUtil->Display_Fill(COLOR_BLACK);
            _carrUtil->Display_PrintCentered("SETTINGS", 30, 2, COLOR_WHITE);

            _carrUtil->Display_Print("DEVICE ID: " + _devId, 30, 60, 1, COLOR_WHITE);
            _carrUtil->Display_Print("HEARTBEAT: " + String(heartbeatIntervalMs / 1000) + " seconds", 30, 75, 1, COLOR_WHITE);
            _carrUtil->Display_Print("DATA: " + String(DATA_INTERVAL_MS / 1000) + " seconds", 30, 90, 1, COLOR_WHITE);
            _carrUtil->Display_Print("DASHBOARD: " + String(SERVER_IP) + ":" + String(DASHBOARD_PORT), 30, 105, 1, COLOR_WHITE);
            _carrUtil->Display_Print("DATABASE: " + String(SERVER_IP) + ":" + String(DB_API_PORT), 30, 120, 1, COLOR_WHITE);
            _carrUtil->Display_Print("WIFI SSID: " + String(WIFI_SSID), 30, 135, 1, COLOR_WHITE);

            _carrUtil->Display_Print("<- BACK", 30, 160, 1, COLOR_WHITE);
        }

        if (screen != SETTINGS)
        {
            lastScreen = screen;
        }

        updateScreen = false;
    }

    if (screen == SETTINGS)
    {
        if (_carrUtil->Button_PressDown(TOUCH0))
        {
            screen = lastScreen;
            updateScreen = true;
        }
    }
    else
    {
        if (_carrUtil->Button_PressDown(TOUCH4))
        {
            screen = static_cast<DeviceScreen>((screen + 1) % 5);
            updateScreen = true;
        }
        else if (_carrUtil->Button_PressDown(TOUCH0))
        {
            screen = static_cast<DeviceScreen>((screen - 1 + 5) % 5);
            updateScreen = true;
        }
        else if (_carrUtil->Button_PressDown(TOUCH2))
        {
            screen = SETTINGS;
            updateScreen = true;
        }
    }

    if (now - lastHeartbeatMs >= heartbeatIntervalMs)
    {
        if (_dataTransmit.sendHeartbeat(_devId))
        {
            lastHeartbeatMs = now;
        }
        else
        {
            return HEARTBEAT_ERROR;
        }
    }

    if (now - lastdataTransmissionMs >= DATA_INTERVAL_MS)
    {
        lastdataTransmissionMs = now;

        if (!_dataTransmit.sendData(_devId, sensorData.temperature, sensorData.humidity, sensorData.pressure, sensorData.light, sensorData.moisture)) 
        {
            return DATA_ERROR;
        }
    }

    if (screen == ALL && now - lastTimeUpdateMs >= timeUpdateMs)
    {
        writeRemainingTime();
        lastTimeUpdateMs = now;
    }

    return CONNECTED;
}

DeviceState handleHeartbeatError(unsigned long now)
{
    if (lastState != HEARTBEAT_ERROR)
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 60, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("HEARTBEAT ERROR", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS 02 TO RETRY", 130, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH2))
    {
        _carrUtil->Display_Fill(COLOR_BLUE);
        _carrUtil->Display_PrintCentered("RETRYING HEARTBEAT", 105, 2, COLOR_WHITE);

        if (!_dataTransmit.sendHeartbeat(_devId))
        {
            lastHeartbeatMs = now;
            return CONNECTED;
        }
        else
        {
            return HEARTBEAT_ERROR;
        }
    }
}

DeviceState handleDataError(SensorData sensorData)
{
    if (lastState != DATA_ERROR)
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 60, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("DATA TRANSMISSION FAILED", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS (02) TO RETRY", 130, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH3))
    {
        _carrUtil->Display_Fill(COLOR_BLUE);
        _carrUtil->Display_PrintCentered("RETRYING DATA TRANSMISSION", 110, 2, COLOR_WHITE);

        if (_dataTransmit.sendData(_devId, sensorData.temperature, sensorData.humidity, sensorData.pressure, sensorData.light, sensorData.moisture))
        {
            return CONNECTED;
        }
        else
        {
            return DATA_ERROR;
        } 
    }
}

