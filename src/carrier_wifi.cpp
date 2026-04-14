#include <carrier_wifi.h>
#include <config.h>

CarrierWiFi::CarrierWiFi()
{
}

bool CarrierWiFi::Init(CarrierUtilities carrUtil, uint16_t timeoutMs)
{
    _carrUtil = &carrUtil;
    return Connect(timeoutMs);
}

bool CarrierWiFi::Connect(uint16_t timeoutMs)
{
    uint8_t retries = 3;
    bool connected = false;

    for (uint8_t i = 0; i < retries; i++)
    {
        _carrUtil->Display_Fill(COLOR_BLACK);
        _carrUtil->Display_PrintLn("Connecting to WiFi", 50, 60, 1, COLOR_WHITE);
        _carrUtil->Display_PrintLn("SSID: " + String(WIFI_SSID), 50, 90, 1, COLOR_WHITE);
        _carrUtil->Display_PrintLn("PASS: " + String(WIFI_PASS), 50, 100, 1, COLOR_WHITE);

        WiFi.begin(WIFI_SSID, WIFI_PASS);
        unsigned long startMs = millis();

        while (millis() - startMs <= timeoutMs)
        {
            delay(timeoutMs / 40);
            _carrUtil->Display_PrintDefault(".", 1, COLOR_WHITE);

            if (IsConnected())
            {
                connected = true;
                _carrUtil->Display_PrintLn("WiFi Connection: OK", 50, 110, 1, COLOR_GREEN);
                _carrUtil->Display_PrintLn(WiFi.localIP().toString(), 50, 130, 1, COLOR_WHITE);
                break;
            }
        }

        if (connected)
        {
            break;
        }
        else
        {
            _carrUtil->Display_PrintLn("Failed to connect to WiFi...", 50, 120, 1, COLOR_RED);
            _carrUtil->Display_PrintLn("Retrying " + String(i + 1) + "/" + String(retries) + "...", 50, 140, 1, COLOR_WHITE);
        }
    }

    return connected;
}

bool CarrierWiFi::IsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void CarrierWiFi::SetToken(String token)
{
    _token = token;
}

String CarrierWiFi::GetDeviceID()
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

bool CarrierWiFi::PostAsJson(const char* endpoint, String jsonBody, String& response, const char* ip, uint16_t port)
{
    uint8_t retries = 3;

    for (uint8_t i = 0; i < retries; i++)
    {
        if (!_client.connect(ip, port))
        {
            Serial.println("-> FAILED");
            delay(750);
            return false;
        }

        Serial.println(String(ip) + ":" + String(port) + String(endpoint));

        String constructedPost = "POST " + String(endpoint) + " HTTP/1.1";

        Serial.println("-> " + constructedPost);

        _client.println(constructedPost);
        _client.println("Host: " + String(ip));
        _client.println("Content-Type: application/json");

        if (_token.length() > 0)
        {
            _client.println("Authorization: Bearer " + _token);
        }
        else
        {
            Serial.println("No token found - Sending POST without token...");
        }

        _client.print("Content-Length: ");
        _client.println(jsonBody.length());
        _client.println("Connection: close");
        _client.println();
        _client.print(jsonBody);

        String statusLine = _client.readStringUntil('\n');
        response = _readResponseBody();
        
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

        Serial.println("Idx_200" + String(idx_200));
        Serial.println("Idx_201" + String(idx_201));
        
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

String CarrierWiFi::_readResponseBody()
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
