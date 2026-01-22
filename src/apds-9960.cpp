#include <apds-9960.h>

APDS_9960::APDS_9960(MKRIoTCarrier& carrier)
    : _carrier(carrier)
{
}

bool APDS_9960::getLight(int& light)
{
    uint16_t timeoutMs = 3000;

    unsigned long startMs = millis();

    while (!_carrier.Light.colorAvailable())
    {
        delay(100);

        if (millis() - startMs >= timeoutMs)
        {
            return false;
        }
    }

    int r,g,b,c;
    _carrier.Light.readColor(r,g,b,light);
    return !isnan(light);
}