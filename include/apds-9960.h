#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

class APDS_9960 
{
public: 
    APDS_9960(MKRIoTCarrier& carrier);
    bool getLight(int& light);

private:
    MKRIoTCarrier& _carrier;
};