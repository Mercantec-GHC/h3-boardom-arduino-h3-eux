#pragma once

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

enum DisplayRotation {
    ROTATION_0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270
};


class CarrierUtilities {
public:
    CarrierUtilities(MKRIoTCarrier& carrier);

    // ------ Display Utilities ------
    Adafruit_ST7789& Display();
    void Display_Fill(uint32_t color);
    void Display_SetRotation(DisplayRotation rotation);
    void Display_Print(String text, uint8_t x, uint8_t y, uint8_t size, uint32_t color);
    void Display_PrintDefault(String text, uint8_t size, uint32_t color);
    void Display_PrintLn(String text, uint8_t x, uint8_t y, uint8_t size, uint32_t color);
    void Display_PrintCentered(String text, uint8_t y, uint8_t size, uint32_t color);

    // ------ LED Utilities ------
    void LED_Set(uint8_t ledIndex, uint32_t color);
    void LED_SetAll(uint32_t color);
    void LED_Clear(uint8_t ledIndex);
    void LED_ClearAll();

    // ------ Button Utilities ------
    bool Button_PressUp(touchButtons button);
    bool Button_PressDown(touchButtons button);

private:
    MKRIoTCarrier& _carrier;
    Adafruit_ST7789& _display;

    const uint8_t _displayW = 240;
    const uint8_t _displayH = 240;
};
