#include <state_logic.h>
#include <config.h>
#include <data_transmission.h>
#include <wifi_handle.h>

DataTransmitter _dataTransmit;

uint32_t heartbeatIntervalMs = 100000;
unsigned long lastHeartbeatMs = 0;

DeviceState lastState = ERROR;

DeviceScreen screen = ALL;
DeviceScreen lastScreen = LIGHT;
DeviceScreen lastDataScreen = ALL;

unsigned long lastdataTransmissionMs = 0;

const char* jwtFilename = "jwt.txt";

uint16_t timeUpdateMs = 1000;
unsigned long lastTimeUpdateMs = 0;

CarrierUtilities* _carrUtil;
String _devId;

void state_init(CarrierUtilities& carrUtil, String devId)
{
    _carrUtil = &carrUtil;
    _devId = devId;

    randomSeed(analogRead(A6) ^ micros() ^ millis());
    heartbeatIntervalMs = random(60000, 180000); // Random interval between 1 and 3 minutes
}

void writeArrows()
{
    _carrUtil->Display_Print("<-", 30, 160, 2, COLOR_WHITE);
    _carrUtil->Display_Print("->", 190, 160, 2, COLOR_WHITE);
}

DeviceState handleToken()
{
    String token = _carrUtil->SD_Read(jwtFilename);

    if (token.length() > 0)
    {
        _dataTransmit.setJwtToken(token);
        return CONNECTED;
    }
    
    return TOKEN_ERROR;
}

void writeRemainingTime()
{
    uint16_t secondsRemaining = (DATA_INTERVAL_MS - (millis() - lastdataTransmissionMs)) / 1000;
        
    _carrUtil->Display_FillRect(0, 140, 240, 10, COLOR_DARK_GREEN);
    _carrUtil->Display_PrintCentered("SENDING DATA IN: " + String(secondsRemaining) + " SECONDS", 140, 1, COLOR_WHITE);
}

void saveLastState(DeviceState newState)
{
    lastState = newState;
}

DeviceState handleStartup()
{
    DeviceState tempState = DISCONNECTED;

    if (_dataTransmit.sendHeartbeat(_devId))
    {
        tempState = CONNECTED;
    }

    if (tempState == CONNECTED)
    {
        tempState = handleToken();
    }
    
    return tempState;
}

DeviceState handleRetrieveToken(String* outToken)
{
    String token = _carrUtil->SD_Read(jwtFilename);

    if (token.length() < 1)
    {
        return TOKEN_ERROR;
    }

    *outToken = token;

    return CONNECTED;
}

DeviceState handleDisconnected()
{
    if (lastState != DISCONNECTED)
    {
        _carrUtil->Display_Fill(COLOR_BLUE);
        _carrUtil->Display_PrintCentered(_devId, 38, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("DISCONNECTED", 90, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS (04) TO CONNECT TO:", 130, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered(String(DASHBOARD_SERVER_IP), 145, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH4))
    {
        _carrUtil->Display_Fill(COLOR_BLUE);
        _carrUtil->Display_PrintCentered(_devId, 38, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("CONNECTING...", 110, 2, COLOR_WHITE);

        if (_dataTransmit.connectDashboard(_devId))
        {
            if (_dataTransmit.sendHeartbeat(_devId))
            {
                return CONNECTED;
            }

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

DeviceState handleConnected(SensorData sensorData, bool& updateScreen)
{
    if (updateScreen)
    {
        if (lastScreen != screen || lastState == HEARTBEAT_ERROR || lastState == DATA_ERROR)
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
        }

        if (screen == ALL)
        {
            _carrUtil->Display_PrintCentered("ALL DATA", 50, 2, COLOR_WHITE);
            _carrUtil->Display_PrintCentered("TEMP | HUM | PRES | LIGHT", 85, 1, COLOR_WHITE);
            _carrUtil->Display_FillPrintCentered(String(sensorData.Temperature, 2) + " C | " + String(sensorData.Humidity, 2) + "% | " + String(sensorData.Pressure, 2) + " hPa | " + String(sensorData.Light), 110, 1, COLOR_WHITE);
            writeArrows();
            writeRemainingTime();
        }
        else if (screen == TEMPERATURE)
        {
            _carrUtil->Display_PrintCentered("TEMPERATURE", 70, 2, COLOR_WHITE);
            _carrUtil->Display_FillPrintCentered(String(sensorData.Temperature, 2) + " C", 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == HUMIDITY)
        {
            _carrUtil->Display_PrintCentered("HUMIDITY", 70, 2, COLOR_WHITE);
            _carrUtil->Display_FillPrintCentered(String(sensorData.Humidity, 2) + " %", 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == PRESSURE)
        {
            _carrUtil->Display_PrintCentered("PRESSURE", 70, 2, COLOR_WHITE);
            _carrUtil->Display_FillPrintCentered(String(sensorData.Pressure, 2) + " hPa", 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == LIGHT)
        {
            _carrUtil->Display_PrintCentered("LIGHT", 70, 2, COLOR_WHITE);
            _carrUtil->Display_FillPrintCentered(String(sensorData.Light), 110, 3, COLOR_WHITE);
            writeArrows();
        }
        else if (screen == SETTINGS && lastScreen != SETTINGS)
        {
            _carrUtil->Display_Fill(COLOR_BLACK);
            _carrUtil->Display_PrintCentered("SETTINGS", 30, 2, COLOR_WHITE);

            _carrUtil->Display_Print("DEVICE ID: " + _devId, 30, 60, 1, COLOR_WHITE);
            _carrUtil->Display_Print("HEARTBEAT: " + String(heartbeatIntervalMs / 1000) + " seconds", 30, 75, 1, COLOR_WHITE);
            _carrUtil->Display_Print("DATA: " + String(DATA_INTERVAL_MS / 1000) + " seconds", 30, 90, 1, COLOR_WHITE);
            
            if (String(DASHBOARD_SERVER_IP).length() < 20 || String(API_SERVER_IP).length() < 20)
            {
                _carrUtil->Display_Print("DASHBOARD: " + String(DASHBOARD_SERVER_IP) + ":" + String(DASHBOARD_PORT), 30, 105, 1, COLOR_WHITE);
                _carrUtil->Display_Print("DATABASE: " + String(API_SERVER_IP) + ":" + String(API_PORT), 30, 120, 1, COLOR_WHITE);
            }
            else
            {
                _carrUtil->Display_Print("DASHBOARD: TOO LONG", 30, 105, 1, COLOR_WHITE);
                _carrUtil->Display_Print("DATABASE: TOO LONG", 30, 120, 1, COLOR_WHITE);
            }

            _carrUtil->Display_Print("WIFI SSID: " + String(WIFI_SSID), 30, 135, 1, COLOR_WHITE);

            _carrUtil->Display_Print("<- BACK", 30, 160, 1, COLOR_WHITE);
            _carrUtil->Display_Print("DISCONNECT ->", 135, 160, 1, COLOR_WHITE);
        }
        else if (screen == CONFIRM_DISCONNECT)
        {
            _carrUtil->Display_Fill(COLOR_RED);
            _carrUtil->Display_PrintCentered("CONFIRM", 60, 2, COLOR_WHITE);
            _carrUtil->Display_PrintCentered("DISCONNECT?", 100, 2, COLOR_WHITE);


            _carrUtil->Display_Print("<- CANCEL", 30, 160, 1, COLOR_WHITE);
            _carrUtil->Display_Print("CONFIRM ->", 150, 160, 1, COLOR_WHITE);
        }

        if (screen != SETTINGS && screen != CONFIRM_DISCONNECT)
        {
            lastDataScreen = screen;
        }


        lastScreen = screen;
        
        updateScreen = false;
    }

    if (screen == SETTINGS)
    {
        if (_carrUtil->Button_PressDown(TOUCH0))
        {
            screen = lastDataScreen;
            updateScreen = true;
        }

        if (_carrUtil->Button_PressDown(TOUCH4))
        {
            screen = CONFIRM_DISCONNECT;
            updateScreen = true;
        }
    }
    else if (screen == CONFIRM_DISCONNECT)
    {
        if (_carrUtil->Button_PressDown(TOUCH0))
        {
            screen = SETTINGS;
            updateScreen = true;
        }
        else if (_carrUtil->Button_PressDown(TOUCH4))
        {
            screen = ALL;
            updateScreen = true;
            return DISCONNECTED;
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

    unsigned long now = millis();

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

        if (!_dataTransmit.sendData(_devId, sensorData.Temperature, sensorData.Humidity, sensorData.Pressure, sensorData.Light, sensorData.Moisture)) 
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

DeviceState handleHeartbeatError()
{
    if (lastState != HEARTBEAT_ERROR)
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 60, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("HEARTBEAT ERROR", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS (04) TO RETRY", 130, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH4))
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 38, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("RETRYING HEARTBEAT", 105, 2, COLOR_WHITE);

        unsigned long now = millis();

        if (_dataTransmit.sendHeartbeat(_devId))
        {
            lastHeartbeatMs = now;
            return CONNECTED;
        }
        else
        {
            lastState = ERROR;
            handleHeartbeatError();
        }
    }

    return HEARTBEAT_ERROR;
}

DeviceState handleDataError(SensorData sensorData)
{
    if (lastState != DATA_ERROR)
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 38, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("DATA TRANSMISSION", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("FAILED", 130, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS (04) TO RETRY", 185, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH4))
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 38, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("RETRYING DATA", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("TRANSMISSION", 130, 2, COLOR_WHITE);

        if (_dataTransmit.sendData(_devId, sensorData.Temperature, sensorData.Humidity, sensorData.Pressure, sensorData.Light, sensorData.Moisture))
        {
            return CONNECTED;
        }
        else
        {
            lastState = ERROR;
            handleDataError(sensorData);
        }
    }

    return DATA_ERROR;
}

DeviceState handleTokenError()
{
    if (lastState != TOKEN_ERROR)
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 38, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("JWT TOKEN", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("ERROR", 130, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS (04) TO RETRY", 185, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH4))
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 38, 1, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("RETRYING TOKEN", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("RETRIEVAL", 130, 2, COLOR_WHITE);

        lastState = ERROR;

        DeviceState tempState = handleToken();

        if (tempState == CONNECTED)
        {
            if (_dataTransmit.sendHeartbeat(_devId))
            {
                lastHeartbeatMs = millis();
                return CONNECTED;
            }
            else
            {
                lastState = ERROR;
                handleHeartbeatError();
                return HEARTBEAT_ERROR;
            }
        }
    }

    return TOKEN_ERROR;
}

DeviceState handleWifiError()
{
    if (lastState != WIFI_ERROR)
    {
        _carrUtil->Display_Fill(COLOR_RED);
        _carrUtil->Display_PrintCentered(_devId, 60, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("WIFI CONNECTION LOST", 110, 2, COLOR_WHITE);
        _carrUtil->Display_PrintCentered("PRESS (02) TO RETRY WIFI CONNECTION", 130, 1, COLOR_WHITE);
    }

    if (_carrUtil->Button_PressDown(TOUCH2))
    {
        if (wifi_Connect(3500))
        {
            return CONNECTED;
        }
    }

    return WIFI_ERROR;
}



