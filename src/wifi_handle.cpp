#include <wifi_handle.h>
#include <WiFiNINA.h>
#include <carrier_utilities.h>
#include <config.h>

CarrierUtilities* _wifi_carrUtil;
WiFiClient _client;
String _jwtToken;

bool wifi_Init(CarrierUtilities carrUtil, uint16_t timeoutMs, String jwtToken) 
{
    _wifi_carrUtil = &carrUtil;
    _jwtToken = jwtToken;

    return wifi_Connect(timeoutMs);
}

bool wifi_Connect(uint16_t timeoutMs)
{
    uint8_t retries = 3;
    bool connected = false;

     for (uint8_t i = 0; i < retries; i++)
    {
        _wifi_carrUtil->Display_Fill(ST7735_BLACK);
        _wifi_carrUtil->Display_PrintLn("Connecting to WIFI", 50, 60, 1, ST7735_WHITE);
        _wifi_carrUtil->Display_Print("SSID: ", 50, 90, 1, ST7735_WHITE);
        _wifi_carrUtil->Display_PrintLn(WIFI_SSID, 100, 90, 1, ST7735_WHITE);
        _wifi_carrUtil->Display_Print("PASS: ", 50, 100, 1, ST7735_WHITE);
        _wifi_carrUtil->Display_PrintLn(WIFI_PASS, 100, 100, 1, ST7735_WHITE);

        WiFi.begin(WIFI_SSID, WIFI_PASS);
        int startMs = millis();

        while (millis() - startMs <= timeoutMs)
        {
            delay(timeoutMs / 40);
            _wifi_carrUtil->Display_PrintDefault(".", 1, ST7735_WHITE);

            if (WiFi.status() == WL_CONNECTED) 
            {
                connected = true;
                Serial.println("WIFI Connection: OK");
                Serial.println(WiFi.localIP().toString());
                _wifi_carrUtil->Display_PrintLn("WIFI Connection: OK", 50, 110, 1, ST7735_GREEN);
                _wifi_carrUtil->Display_PrintLn(WiFi.localIP().toString(), 50, 130, 1, ST7735_WHITE);
                break;
            }
        }

        if (connected) 
        {
            break;
        }
        else
        {
            _wifi_carrUtil->Display_PrintLn("Failed to connect to WiFi...", 50, 120, 1, ST7735_RED);
            _wifi_carrUtil->Display_PrintLn("Retrying" + String(i + 1) + "/" + String(retries) + "...", 50, 140, 1, ST7735_WHITE);
            delay(750);
        }
    }

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

    byte b1 = mac[0] ^ mac[3];
    byte b2 = mac[1] ^ mac[4];
    byte b3 = mac[2] ^ mac[5];

    char buf[20];
    snprintf(buf, sizeof(buf), "OPLA_%02X%02X%02X", b1, b2, b3);

    return String(buf);
}

String readResponseBody()
{
    while (_client.connected())
    {
        String line = _client.readStringUntil('\n');

        if (line == "\r" || line.length() == 0) 
        {
            break;
        }
    }

    return _client.readString();
}

bool wifi_HttpPost(const char* endpoint, String jsonBody, String& response, const char* server_ip, uint16_t server_port)
{
    uint8_t retries = 3;

    for (uint8_t i = 0; i < retries; i++)
    {

        if (!_client.connect(server_ip, server_port))
        {
            Serial.println(" -> FAILED");
            delay(750);
            return false;
        }

        Serial.println(String(server_ip) + ":" + String(server_port) + String(endpoint));

        String constructedPost = "POST " + String(endpoint) + " HTTP/1.1";

        Serial.println("-> " + constructedPost);

        _client.println(constructedPost);
        _client.println("Host: " + String(server_ip));
        _client.println("Content-Type: application/json");

        if (_jwtToken.length() > 0)
        {
            _client.println("Authorization: Bearer " + _jwtToken);
        }

        _client.print("Content-Length: ");
        _client.println(jsonBody.length());
        _client.println("Connection: close");
        _client.println();
        _client.print(jsonBody);

        String statusLine = _client.readStringUntil('\n');
        response = readResponseBody();
        Serial.println(response);
        _client.stop();

        int jsonStart = response.indexOf('{');

        if (jsonStart >= 0)
        {
            response = response.substring(jsonStart);
        }

        int16_t idx_200 = statusLine.indexOf("200");
        int16_t idx_201 = statusLine.indexOf("201");

        bool success = false;

        Serial.println("Idx_200: " + String(idx_200));
        Serial.println("Idx_201: " + String(idx_201));

        if (idx_200 >= 0 || idx_201 >= 0)
        {
            success = true;
        }

        Serial.println(statusLine);
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