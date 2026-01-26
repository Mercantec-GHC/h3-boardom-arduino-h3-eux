#include <data-transmission.h>
#include <Arduino_JSON.h>
#include <config.h>
#include <wifi_handle.h>

DataTransmitter::DataTransmitter()
{
}

bool DataTransmitter::sendHeartbeat(String devId)
{
    JSONVar doc;
    doc["deviceId"] = devId;
    String json = JSON.stringify(doc);

    String postResponse;

    if (wifi_HttpPost("/api/heartbeat", json, postResponse, SERVER_IP, DASHBOARD_PORT))
    {
        JSONVar res = JSON.parse(postResponse);

        if ((bool)res["success"])
        {
            return true;
        }
    }

    return false;
}

bool DataTransmitter::sendData(String devId, float temperature, float humidity, float pressure, int light, float moisture)
{
    JSONVar doc;
    doc["deviceId"] = devId;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["pressure"] = pressure;
    doc["light"] = light;
    doc["moisture"] = moisture;
    String json = JSON.stringify(doc);

    String postResponse;

    if (wifi_HttpPost("/Data/sensorData", json, postResponse, SERVER_IP, DASHBOARD_PORT))
    {
        JSONVar res = JSON.parse(postResponse);

        if (String((const char*) res["state"]) == devId)
        {
            return true;
        }
    }

    return false;
}