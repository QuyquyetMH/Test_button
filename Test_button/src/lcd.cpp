#include "LCD.h"

LCD::LCD(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows) 
    : lcd(lcd_addr, lcd_cols, lcd_rows) {}

void LCD::begin() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void LCD::displayCounts(int pressCount, int setupCount) {
    lcd.setCursor(0, 0);  // Hiển thị trên dòng đầu tiên
    lcd.print("Press: ");
    lcd.print(pressCount);
    lcd.setCursor(0, 1);  // Hiển thị trên dòng thứ hai
    lcd.print("Setup: ");
    lcd.print(setupCount);
}

void LCD::displayStatus(const char* status) {
    lcd.setCursor(10, 0);  // Hiển thị trạng thái ở góc phải dòng đầu tiên
    lcd.print(status);
}

void LCD::displayError(int errorCode) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR ");
    lcd.print(errorCode);
}

