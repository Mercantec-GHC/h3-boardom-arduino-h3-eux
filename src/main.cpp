#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include <config.h>
#include <carrier_utilities.h>

MKRIoTCarrier carrier;

CarrierUtilities carr(carrier);

void setup() 
{
    Serial.begin(9600);

    USING_CARRIER_CASE ? carrier.withCase() : carrier.noCase();

    carrier.begin();

    carr.Display_SetRotation(ROTATION_90);
    carr.Display_Fill(ST7735_BLUE);
    carr.Display_PrintCentered("Hello World", 100, 2, ST7735_WHITE);
    carr.Display_PrintLn("The utilities are working!", 50, 150, 1, ST7735_WHITE);

    carr.LED_SetAll(ST7735_BLUE);
}

void loop() 
{
  
}

