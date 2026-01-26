#include <Arduino.h>

class DataTransmitter
{
public:
    DataTransmitter();
    bool sendHeartbeat(String devId);
    bool sendData(String devId, float temperature, float humidity, float pressure, int light, float moisture);
};