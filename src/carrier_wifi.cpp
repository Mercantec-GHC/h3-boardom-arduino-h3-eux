#include <carrier_wifi.h>
#include <config.h>

String _addLine(String name, uint16_t length)
{
    uint16_t dashLength = length - (name.length() + 2);
    uint16_t leftDashes = dashLength / 2;
    uint16_t rightDashes = dashLength - leftDashes;

    String leftStr = "";
    for (uint8_t i = 0; i < leftDashes; i++)
    {
        leftStr += "-";
    }

    String rightStr = "";
    for (uint8_t i = 0; i < rightDashes; i++)
    {
        rightStr += "-";
    }

    return leftStr + " " + name + " " + rightStr;
}

String _formatJson(String json)
{
    String formatted = "";
    uint8_t indent = 0;
    bool inString = false;
    
    for (uint16_t i = 0; i < json.length(); i++)
    {
        char c = json.charAt(i);
        char prev = (i > 0) ? json.charAt(i - 1) : '\0';
        
        if (c == '"' && prev != '\\')
        {
            inString = !inString;
        }
        
        if (!inString)
        {
            if (c == '{' || c == '[')
            {
                formatted += c;
                formatted += "\n";
                indent++;
                for (uint8_t j = 0; j < indent; j++) formatted += "  ";
            }
            else if (c == '}' || c == ']')
            {
                formatted += "\n";
                indent--;
                for (uint8_t j = 0; j < indent; j++) formatted += "  ";
                formatted += c;
            }
            else if (c == ',')
            {
                formatted += c;
                formatted += "\n";
                for (uint8_t j = 0; j < indent; j++) formatted += "  ";
            }
            else if (c == ':')
            {
                formatted += c;
                formatted += " ";
            }
            else if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
            {
                formatted += c;
            }
        }
        else
        {
            formatted += c;
        }
    }
    
    return formatted;
}

CarrierWiFi::CarrierWiFi()
{
}

bool CarrierWiFi::Init(CarrierUtilities& carrUtil, uint16_t timeoutMs)
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

String CarrierWiFi::GetToken()
{
    return _token;
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

        Serial.println(_addLine("POST REQUEST", 90));
        Serial.println(_addLine("FULL URL", 40));
        Serial.println(String(ip) + ":" + String(port) + String(endpoint));

        String constructedPost = "POST " + String(endpoint) + " HTTP/1.1";

        _client.println(constructedPost);
        _client.println("Host: " + String(ip));
        _client.println("Content-Type: application/json");

        Serial.println(_addLine("TOKEN", 40));

        if (_token.length() > 0)
        {
            Serial.println("TRUE");
            _client.println("Authorization: Bearer " + _token);
        }
        else
        {
            Serial.println("FALSE");
        }

        _client.print("Content-Length: ");
        _client.println(jsonBody.length());
        _client.println("Connection: close");
        _client.println();
        _client.print(jsonBody);

        Serial.println(_addLine("REQUEST BODY", 40));
        Serial.println(_formatJson(jsonBody));

        String statusLine = _client.readStringUntil('\n');
        statusLine.trim();
        response = _readResponseBody();
        
        Serial.println(_addLine("RESPONSE BODY", 40));

        String trimmedResponse = response;

        trimmedResponse.trim();
        
        int16_t idx_oBracket =  trimmedResponse.indexOf("{");
        int16_t idx_cBracket =  trimmedResponse.indexOf("}");

        trimmedResponse.remove(idx_cBracket + 1);
        trimmedResponse.remove(0, idx_oBracket);

        Serial.println(_formatJson(trimmedResponse));

        _client.stop();

        int jsonStart = response.indexOf('{');

        if (jsonStart >= 0) 
        {
            response = response.substring(jsonStart);
        }

        int16_t idx_200 = statusLine.indexOf("200");
        int16_t idx_201 = statusLine.indexOf("201");

        bool success = false;

        if (idx_200 >= 0 || idx_201 >= 0)
        {
            success = true;
        }

        
        Serial.println(_addLine("STATUSLINE", 40));
        Serial.println(statusLine + " -> " + (success ? "OK" : "FAILED")); 
        
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
