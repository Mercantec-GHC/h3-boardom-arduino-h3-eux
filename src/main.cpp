#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include <carrier_utilities.h>
#include <config.h>
#include <wifi_handle.h>

MKRIoTCarrier carrier;

CarrierUtilities carr(carrier);

void setup() 
{
    Serial.begin(9600);

    USING_CARRIER_CASE ? carrier.withCase() : carrier.noCase();

    carrier.begin();

    carr.Display_SetRotation(ROTATION_0);

    while (!wifi_Init(carr, 3500)) 
    {
        carr.Display_Fill(ST7735_RED);
        carr.Display_PrintCentered("WIFI FAILED", 100, 2, ST7735_WHITE);
        carr.Display_PrintCentered("PRESS (04) TO TRY AGAIN", 130, 1, ST7735_WHITE);

        while (1) 
        {
            if (carr.Button_PressDown(TOUCH4)) 
            {
                break;
            }
        }
    }
}

void loop() 
{
  
}

