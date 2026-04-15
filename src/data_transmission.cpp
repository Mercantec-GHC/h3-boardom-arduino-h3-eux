#include <data_transmission.h>
#include <Arduino_JSON.h>
#include <config.h>

double round2(float v)
{
    return (double)((int)(v * 100.0 + 0.5)) / 100.0;
}

DataTransmitter::DataTransmitter()
{
}

void DataTransmitter::Init(CarrierUtilities& carrUtil, CarrierWiFi& carrWifi, const char* jwtFilename)
{
    _carrUtil = &carrUtil;
    _carrWifi = &carrWifi;
    _jwtFilename = jwtFilename;
}

bool DataTransmitter::ConnectDashboard(String devId)
{
    JSONVar doc;
    doc["deviceId"] = devId;
    String json = JSON.stringify(doc);

    String postResponse;

    if (_carrWifi->PostAsJson("/api/connect", json, postResponse, DASHBOARD_SERVER_IP, DASHBOARD_PORT))
    {
        JSONVar res = JSON.parse(postResponse);

        if ((bool)res["success"])
        {
            return true;
        }
    }

    return false;
}

bool DataTransmitter::SendHeartbeat(String devId)
{
    JSONVar doc;
    doc["deviceId"] = devId;
    String json = JSON.stringify(doc);

    String postResponse;
    String token;

    if (_carrWifi->PostAsJson("/Device/heartbeat", json, postResponse, API_SERVER_IP, API_PORT))
    {
        JSONVar res = JSON.parse(postResponse);

        if ((bool)res["success"])
        {
            token = (const char*)res["accessToken"];

            _writeJwtToken(token);

            return true;
        }
    }

    return false;
}

bool DataTransmitter::SendData(String devId, float temperature, float humidity, float pressure, int light, float moisture)
{
    JSONVar doc;
    doc["deviceId"] = devId;
    doc["temperature"] = round2(temperature);
    doc["humidity"] = round2(humidity);
    doc["pressure"] = round2(pressure);
    doc["light"] = light;
    doc["moisture"] = round2(moisture);
    
    String json = JSON.stringify(doc);

    Serial.println(json);

    String postResponse;

    if (_carrWifi->PostAsJson("/Data/sensorData", json, postResponse, API_SERVER_IP, API_PORT))
    {
        JSONVar res = JSON.parse(postResponse);

        if (String((const char*)res["error"]) != null)
        {
            return false;
        }

        if (String((const char*)res["message"]).indexOf("recorded") > 0 && String((const char*)res["deviceId"]) == devId)
        {
            return true;
        }
    }

    return false;
}

void DataTransmitter::_writeJwtToken(String token)
{
    if (token.length() > 0)
    {
        bool success = _carrUtil->SD_WriteOver(_jwtFilename, token);

        if (success)
        {
            Serial.println("JWT Saved to SD: TRUE");
        }
        else
        {
            Serial.println("JWT Saved to SD: FALSE");
        }
    
        SetJwtToken(token);
    }
}


void DataTransmitter::SetJwtToken(String token)
{
    _carrWifi->SetToken(token);
}