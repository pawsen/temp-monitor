#ifndef MENU_H
#define MENU_H

#include "heaterControl.h"
#include <Arduino.h>
#include <Encoder.h>
// #include "lcd.h"

#include <I2C_LCD.h>

class Menu {
public:
    Menu(uint8_t ENCODER_PIN_A, uint8_t ENCODER_PIN_B, uint8_t ENCODER_BUTTON_PIN);
  // void init(uint8_t ENCODER_PIN_A, uint8_t ENCODER_PIN_B,
  //           uint8_t ENCODER_BUTTON_PIN);
    void init();
  void update();
    void displayDefaultScreen(double currentTemp, double targetTemp);

private:
  Encoder encoder;
  int currentMenuIndex = 0;
  bool menuActive = false;
  bool heaterEnabled = false;
  unsigned long lastButtonPress = 0;
  unsigned long lastButtonRelease = 0;
  bool buttonPressed = false;
bool longPressHandled = false;

  void handleMenuNavigation();
  void selectMenuItem();
  void displayMenu();
  void handleLongPress();
  void adjustTargetTemperature();
  void adjustAutoDisable();

  uint8_t ENCODER_PIN_A;
  uint8_t ENCODER_PIN_B;
  uint8_t ENCODER_BUTTON_PIN;
};
// Access the HeaterControl instance from main.ino
extern HeaterControl heaterControl;
extern Menu menu;
extern I2C_LCD lcd;

#endif
