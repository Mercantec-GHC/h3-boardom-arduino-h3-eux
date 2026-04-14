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

void DataTransmitter::Init(CarrierWiFi& carrWifi)
{
    _carrWifi = &carrWifi;
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

    if (_carrWifi->PostAsJson("/Device/heartbeat", json, postResponse, API_SERVER_IP, API_PORT))
    {
        JSONVar res = JSON.parse(postResponse);

        Serial.println(JSON.stringify(res));

        if ((bool)res["success"])
        {
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

        Serial.println(String((const char*) res["deviceId"]));

        if (String((const char*) res["deviceId"]) == devId)
        {
            return true;
        }
    }

    return false;
}

void DataTransmitter::SetJwtToken(String token)
{
    Serial.println("Setting WiFi JWT Token: " + token);
    _carrWifi->SetToken(token);
}