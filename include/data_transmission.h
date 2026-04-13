#include <Arduino.h>

class DataTransmitter
{
public:
    DataTransmitter();
    bool connectDashboard(String devId);
    bool sendHeartbeat(String devId);
    bool sendData(String devId, float temperature, float humidity, float pressure, int light, float moisture);
    void setJwtToken(String token);
};