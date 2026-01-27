#include <wifi_handle.h>
#include <WiFiNINA.h>
#include <carrier_utilities.h>
#include <config.h>

CarrierUtilities* _carrUtil;
WiFiClient client;

bool wifi_Init(CarrierUtilities carrUtil, uint16_t timeoutMs) 
{
    _carrUtil = &carrUtil;

    uint8_t retries = 3;
    bool connected = false;

    for (uint8_t i = 0; i < retries; i++)
    {
        _carrUtil->Display_Fill(ST7735_BLACK);
        _carrUtil->Display_PrintLn("Connecting to WIFI", 50, 60, 1, ST7735_WHITE);
        _carrUtil->Display_Print("SSID: ", 50, 90, 1, ST7735_WHITE);
        _carrUtil->Display_PrintLn(WIFI_SSID, 100, 90, 1, ST7735_WHITE);
        _carrUtil->Display_Print("PASS: ", 50, 100, 1, ST7735_WHITE);
        _carrUtil->Display_PrintLn(WIFI_PASS, 100, 100, 1, ST7735_WHITE);

        WiFi.begin(WIFI_SSID, WIFI_PASS);
        int startMs = millis();

        while (millis() - startMs <= timeoutMs)
        {
            delay(timeoutMs / 40);
            _carrUtil->Display_PrintDefault(".", 1, ST7735_WHITE);

            if (WiFi.status() == WL_CONNECTED) 
            {
                connected = true;
                Serial.println("WIFI Connection: OK");
                Serial.println(WiFi.localIP().toString());
                _carrUtil->Display_PrintLn("WIFI Connection: OK", 50, 110, 1, ST7735_GREEN);
                _carrUtil->Display_PrintLn(WiFi.localIP().toString(), 50, 130, 1, ST7735_WHITE);
                break;
            }
        }

        if (connected) 
        {
            break;
        }
        else
        {
            _carrUtil->Display_PrintLn("Failed to connect to WiFi...", 50, 120, 1, ST7735_RED);
            _carrUtil->Display_PrintLn("Retrying" + String(i + 1) + "/" + String(retries) + "...", 50, 140, 1, ST7735_WHITE);
            delay(750);
        }
    }

    delay(1000);

    return connected;
}

bool wifi_IsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String wifi_GetDeviceID()
{
    byte mac[6];
    WiFi.macAddress(mac);
    char buf[20];
    snprintf(buf, sizeof(buf), "OPLA_%02X%02X%02X", mac[3], mac[4], mac[5]);
    String deviceId = String(buf);
    return deviceId;
}

String readResponseBody()
{
    while (client.connected())
    {
        String line = client.readStringUntil('\n');

        if (line == "\r" || line.length() == 0) 
        {
            break;
        }
    }

    return client.readString();
}

bool wifi_HttpPost(const char* endpoint, String jsonBody, String& response, const char* server_ip, uint16_t server_port)
{
    uint8_t retries = 3;

    for (uint8_t i = 0; i < retries; i++)
    {
        Serial.println("Connecting to server: " + String(server_ip) + ":" + String(server_port));

        if (!client.connect(server_ip, server_port))
        {
            Serial.println(" -> FAILED");
            delay(750);
            return false;
        }

        String constructedPost = "POST " + String(endpoint) + " HTTP/1.1";

        Serial.println("-> " + constructedPost);

        client.println(constructedPost);
        client.println("Host: " + String(server_ip));
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(jsonBody.length());
        client.println("Connection: close");
        client.println();
        client.print(jsonBody);

        String statusLine = client.readStringUntil('\n');
        response = readResponseBody();
        Serial.println(response);
        client.stop();

        int jsonStart = response.indexOf('{');

        if (jsonStart >= 0)
        {
            response = response.substring(jsonStart);
        }

        uint16_t idx_200 = statusLine.indexOf("200");
        uint16_t idx_201 = statusLine.indexOf("201");

        bool success = false;

        if (idx_200 >= 0 || idx_201 >= 0)
        {
            success = true;
        }
        else
        {
            success = false;
        }

        Serial.print(" -> ");
        Serial.println(success ? "OK" : "FAILED");

        if (success)
        {
            return success;
        }

        Serial.println("Retrying " + String(i+1) + "/" + String(retries));
        delay(500);
    }
}