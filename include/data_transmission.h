#include <Arduino.h>
#include <carrier_wifi.h>

class DataTransmitter
{
public:
    DataTransmitter();
    void Init(CarrierUtilities& carrUtil, CarrierWiFi& carrWifi, const char* jwtFilename);
    bool ConnectDashboard(String devId);
    bool SendHeartbeat(String devId);
    bool SendData(String devId, float temperature, float humidity, float pressure, int light, float moisture);
    void SetJwtToken(String token);

private:
    void _writeJwtToken(String token);
    CarrierWiFi* _carrWifi;
    CarrierUtilities* _carrUtil;
    const char* _jwtFilename;
};