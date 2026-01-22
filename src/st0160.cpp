#include <st0160.h>

ST0160::ST0160(MKRIoTCarrier& carrier)
    : _carrier(carrier)
{
}

bool ST0160::getMoisture(int pin, float& moisture)
{
    pinMode(pin, INPUT);

    int readVal = analogRead(pin);
    float voltage = readVal * (3.3 / 4095.0);
    moisture = (voltage / 3.3) * 100;

    return !isnan(moisture);
}   