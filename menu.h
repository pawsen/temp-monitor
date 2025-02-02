#ifndef MENU_H
#define MENU_H

#include "heaterControl.h"
#include "log.h"

#include <Arduino.h>
#include <Encoder.h>
#include <Bounce2.h>
#include <I2C_LCD.h>

class Menu {
public:
    Menu(uint8_t ENCODER_PIN_A, uint8_t ENCODER_PIN_B, uint8_t ENCODER_BUTTON_PIN);
    void init();
  void update();
    void displayDefaultScreen(double currentTemp, double targetTemp);

private:
  Encoder encoder;
  Bounce bounce;

  int currentMenuIndex = 0;
  int lastMenuIndex = -1;
  bool menuActive = false;
  bool adjusting = false;
  bool heaterEnabled = false;
  unsigned long lastButtonPress = 0;
  unsigned long lastButtonRelease = 0;
  bool buttonPressed = false;
  bool longPressHandled = false;

  void handleMenuNavigation();
  void selectMenuItem();
  void displayMenu();
  void handleLongPress();
  void handleShortPress();
  void adjustTargetTemperature();
  void adjustAutoDisable();
  void exitMenu();

  uint8_t ENCODER_PIN_A;
  uint8_t ENCODER_PIN_B;
  uint8_t ENCODER_BUTTON_PIN;
};
// Access the HeaterControl instance from main.ino
extern HeaterControl heaterControl;
extern Menu menu;
extern I2C_LCD lcd;
extern Log logger;

void displayError(Log &logger);
void getHoursAndMinutes(uint32_t autoDisableTime, uint8_t &hours, uint8_t &minutes);
#endif
