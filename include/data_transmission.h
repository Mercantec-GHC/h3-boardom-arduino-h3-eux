#include <Arduino.h>
#include <carrier_wifi.h>

class DataTransmitter
{
public:
    DataTransmitter();
    void Init(CarrierWiFi& carrWifi);
    bool ConnectDashboard(String devId);
    bool SendHeartbeat(String devId);
    bool SendData(String devId, float temperature, float humidity, float pressure, int light, float moisture);
    void SetJwtToken(String token);

private:
    CarrierWiFi* _carrWifi;
};