#include "lcd.h"

// Constructor
LCD::LCD(uint8_t i2cAddress) : lcd(i2cAddress) {}

// Initialize the LCD
void LCD::init() {
    lcd.begin();
    lcd.backlight(); // Turn on the backlight
    lcd.clear();
}

// Display current and target temperatures
void LCD::displayTemperature(double currentTemperature, double targetTemperature) {
    lcd.setCursor(0, 0);
    lcd.print("Current: ");
    lcd.print(currentTemperature, 1); // Display with 1 decimal place
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Target:  ");
    lcd.print(targetTemperature, 1); // Display with 1 decimal place
    lcd.print(" C");
}

// Display heater status
void LCD::displayHeaterStatus(bool heaterOn) {
    lcd.setCursor(0, 2);
    lcd.print("Heater:  ");
    lcd.print(heaterOn ? "ON " : "OFF");
}

// Display time remaining for auto-disable
void LCD::displayAutoDisableTime(uint32_t timeRemaining) {
    lcd.setCursor(0, 3);
    lcd.print("Auto-Off: ");
    if (timeRemaining > 0) {
        uint32_t minutes = timeRemaining / 60000;
        lcd.print(minutes);
        lcd.print(" min");
    } else {
        lcd.print("N/A     ");
    }
}

// Display a menu item
void LCD::displayMenu(const String &menuItem) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Menu: ");
    lcd.print(menuItem);
}

// Clear the LCD
void LCD::clear() {
    lcd.clear();
}
