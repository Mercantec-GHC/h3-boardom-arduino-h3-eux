#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include "carrier_utilities.h"

CarrierUtilities::CarrierUtilities(MKRIoTCarrier& carrier)
    : _carrier(carrier),
      _display(carrier.display)
{
}

// ------------------ Display Utilities ------------------

Adafruit_ST7789& CarrierUtilities::Display() {
    return _display;
}

void CarrierUtilities::Display_Fill(uint32_t color)
{
    _display.fillScreen(color);
}

void CarrierUtilities::Display_SetRotation(DisplayRotation rotation)
{
    _display.setRotation(static_cast<uint8_t>(rotation));
}

void CarrierUtilities::Display_Print(String text, uint8_t x, uint8_t y, uint8_t size, uint32_t color)
{
    _display.setTextSize(size);
    _display.setCursor(x, y);
    _display.setTextColor(color);
    _display.print(text);
}

void CarrierUtilities::Display_PrintLn(String text, uint8_t x, uint8_t y, uint8_t size, uint32_t color)
{
    _display.setTextSize(size);
    _display.setCursor(x, y);
    _display.setTextColor(color);
    _display.println(text);
}

void CarrierUtilities::Display_PrintCentered(String text, uint8_t y, uint8_t size, uint32_t color)
{
    _display.setTextSize(size);
    _display.setTextColor(color);

    uint8_t charWidth = 6 * size;
    uint16_t textWidth = text.length() * charWidth;

    uint8_t x = (_displayW - textWidth) / 2;

    _display.setCursor(x, y);
    _display.print(text);
}

// -------------------- LED Utilities --------------------

void CarrierUtilities::LED_Set(uint8_t ledIndex, uint32_t color)
{
    _carrier.leds.setPixelColor(ledIndex, color);
    _carrier.leds.show();
}

void CarrierUtilities::LED_SetAll(uint32_t color)
{
    for (uint8_t i = 0; i < _carrier.leds.numPixels(); i++)
    {
        _carrier.leds.setPixelColor(i, color);
    }
    _carrier.leds.show();
}

void CarrierUtilities::carrierUtil_LED_Clear(uint8_t ledIndex)
{
    _carrier.leds.setPixelColor(ledIndex, 0);
    _carrier.leds.show();
}

void CarrierUtilities::carrierUtil_LED_ClearAll()
{
    _carrier.leds.clear();
    _carrier.leds.show();
}

// -------------------- Button Utilities --------------------

bool CarrierUtilities::Button_PressUp(touchButtons button)
{
    _carrier.Buttons.update();
    return _carrier.Buttons.onTouchUp(button);
}

bool CarrierUtilities::Button_PressDown(touchButtons button)
{
    _carrier.Buttons.update();
    return _carrier.Buttons.onTouchDown(button);
}

