#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

class ST0160
{
public:
    ST0160(MKRIoTCarrier& carrier);
    bool getMoisture(int pin, float& moisture);
    
private:
    MKRIoTCarrier& _carrier;
};