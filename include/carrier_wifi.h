#pragma once

#include <Arduino.h>
#include <carrier_utilities.h>
#include <WiFiNINA.h>

class CarrierWiFi {
public:
    CarrierWiFi();

    bool Init(CarrierUtilities& carrUtil, uint16_t timeoutMs);
    bool Connect(uint16_t timeoutMs);
    bool IsConnected();
    void SetToken(String token);
    String GetToken();
    String GetDeviceID();
    bool PostAsJson(const char* endpoint, String jsonBody, String& response, const char* ip, uint16_t port);

private:
    String _readResponseBody();

    CarrierUtilities* _carrUtil;
    WiFiClient _client;
    String _token;
};