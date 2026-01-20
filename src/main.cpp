#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include <config.h>

MKRIoTCarrier carrier;

void setup() 
{
    Serial.begin(9600);

    USING_CARRIER_CASE ? carrier.withCase() : carrier.noCase();

    carrier.begin();
}

void loop() 
{
  
}

