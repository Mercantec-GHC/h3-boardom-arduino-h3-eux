#pragma once

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

enum DisplayRotation {
    ROTATION_0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270
};

enum Colors {
    COLOR_BLACK = 0x0000,
    COLOR_WHITE = 0xFFFF,
    COLOR_RED = 0xF800,
    COLOR_GREEN = 0x07E0,
    COLOR_DARK_GREEN = 0x0320,
    COLOR_BLUE = 0x001F,
    COLOR_YELLOW = 0xFFE0,
    COLOR_CYAN = 0x07FF,
    COLOR_MAGENTA = 0xF81F
};


class CarrierUtilities {
public:
    CarrierUtilities();

    // ------ Misc ------
    void Init(bool usingCase);
    MKRIoTCarrier& Get_Carrier();

    // ------ Display Utilities ------
    Adafruit_ST7789& Display();
    void Display_Fill(uint32_t color);
    void Display_FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint32_t color);
    void Display_SetCursor(uint8_t x, uint8_t y);
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
    static MKRIoTCarrier _carrier;
    Adafruit_ST7789& _display;

    const uint8_t _displayW = 240;
    const uint8_t _displayH = 240;
};
