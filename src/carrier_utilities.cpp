#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include "carrier_utilities.h"

MKRIoTCarrier CarrierUtilities::_carrier;

CarrierUtilities::CarrierUtilities()
      :_display(_carrier.display)
{

}

// ------------------ Misc ------------------

void CarrierUtilities::Init(bool usingCase)
{
    usingCase ? _carrier.withCase() : _carrier.noCase();
    _carrier.begin();
}

MKRIoTCarrier& CarrierUtilities::Get_Carrier()
{
    return _carrier;
}

// ------------------ Display Utilities ------------------

Adafruit_ST7789& CarrierUtilities::Display() {
    return _display;
}

void CarrierUtilities::Display_Fill(uint32_t color)
{
    _display.fillScreen(color);
    currentFillColor = color;
}

void CarrierUtilities::Display_FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint32_t color)
{
    _display.fillRect(x, y, width, height, color);
}

void CarrierUtilities::Display_SetCursor(uint8_t x, uint8_t y)
{
    _display.setCursor(x, y);
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

void CarrierUtilities::Display_PrintDefault(String text, uint8_t size, uint32_t color)
{
    _display.setTextSize(size);
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

void CarrierUtilities::Display_FillPrintCentered(String text, uint8_t y, uint8_t size, uint32_t color)
{
    uint8_t height = 8 * size;

    Display_FillRect(0, y, _displayW, height, currentFillColor);
    Display_PrintCentered(text, y, size, color);
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

void CarrierUtilities::LED_Clear(uint8_t ledIndex)
{
    _carrier.leds.setPixelColor(ledIndex, 0);
    _carrier.leds.show();
}

void CarrierUtilities::LED_ClearAll()
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

// -------------------- SD Utilities --------------------

bool CarrierUtilities::SD_Delete(const char* fileName)
{
    if (!fileName || fileName[0] == '\0')
    {
        return false;
    }

    if (SD.exists(fileName))
    {
        SD.remove(fileName);
        
        if (!SD.exists(fileName))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}

bool CarrierUtilities::SD_Write(const char* fileName, String data)
{
    if (!fileName || fileName[0] == '\0')
    {
        return false;
    }

    File f = SD.open(fileName, FILE_WRITE);

    if (!f) 
    {
        return false;
    }

    size_t written = f.print(data);

    f.close();

    return written == data.length();

}

bool CarrierUtilities::SD_WriteOver(const char* fileName, String data)
{
    if (!SD_Delete(fileName))
    {
        return false;
    }

    return SD_Write(fileName, data);
}

String CarrierUtilities::SD_Read(const char* fileName)
{
    if (!fileName || fileName[0] == '\0')
    {
        return "";
    }

    File f = SD.open(fileName, FILE_READ);

    if (!f)
    {
        return "";
    }

    String out;

    while (f.available())
    {
        out += (char)f.read();
    }

    f.close();

    return out;
}   