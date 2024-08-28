#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LCD {
public:
    LCD(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows);

    void begin();
    void displayCounts(int pressCount, int setupCount);
    void displayStatus(const char* status);
    void displayError(int errorCode);

private:
    LiquidCrystal_I2C lcd;
};

#endif
