#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

class BME688 
{
public:
    BME688(MKRIoTCarrier& carrier);
    bool getHumidity(float& humidity);
    bool getTemperature(float& temperature);
    bool getPressure(float& pressure);

private:
    MKRIoTCarrier& _carrier;

};