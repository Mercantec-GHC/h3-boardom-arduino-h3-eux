#include <bme688.h>

BME688::BME688(MKRIoTCarrier& carrier)
    : _carrier(carrier)
{
}

bool BME688::getHumidity(float& humidity)
{
    humidity = _carrier.Env.readHumidity();
    return !isnan(humidity);
}

bool BME688::getTemperature(float& temperature)
{
    temperature = _carrier.Env.readTemperature();
    return !isnan(temperature);
}

bool BME688::getPressure(float& pressure)
{
    pressure = _carrier.Pressure.readPressure();
    return !isnan(pressure);
}