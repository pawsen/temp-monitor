#include "menu.h"

// Constructor with member initializer list for the encoder
Menu::Menu(uint8_t ENCODER_PIN_A, uint8_t ENCODER_PIN_B,
           uint8_t ENCODER_BUTTON_PIN)
    : encoder(ENCODER_PIN_A, ENCODER_PIN_B),
      ENCODER_BUTTON_PIN(ENCODER_BUTTON_PIN) {}

// Initialize button pin and any other hardware
void Menu::init() {
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  bounce.attach(ENCODER_BUTTON_PIN);
  bounce.interval(25);
  Serial.println(F("Menu initialized."));
}

void Menu::exitMenu() {
  menuActive = false;
  lcd.clear();
  Serial.println(F("Exiting menu..."));
}

void Menu::update() {
  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
  bounce.update();

  static bool longPressHandled =
      false; // Track if long press was already handled

  if (bounce.fell()) { // Button just pressed
    longPressHandled = false;
  }

  if (!longPressHandled && bounce.read() == LOW) { // Button is still held
    if (bounce.currentDuration() > 1000) {
      Serial.println("long press");
      handleLongPress();
      longPressHandled = true; // Prevent float execution
    }
  }

  if (bounce.rose() && !longPressHandled) { // Button released before 1000ms
    Serial.println("short press");
    handleShortPress();
  }

  // Handle menu navigation if the menu is active
  if (menuActive) {
    handleMenuNavigation();
  }
}

void Menu::handleMenuNavigation() {
  int encoderPos = encoder.read() / 4;
  static int lastEncoderPos = 0;

  if (encoderPos != lastEncoderPos) {
    if (encoderPos > lastEncoderPos) {
      currentMenuIndex = (currentMenuIndex + 1) % menuItemCount;
    } else if (encoderPos < lastEncoderPos) {
      currentMenuIndex = (currentMenuIndex - 1 + menuItemCount) % menuItemCount;
    }
    lastEncoderPos = encoderPos;
    displayMenu();
  }
}

void Menu::handleShortPress() {
  if (!menuActive) {
    // Enter the menu
    menuActive = true;
    currentMenuIndex = 0;
    displayMenu();
  } else {
    // Select the current menu item
    selectMenuItem();
  }
}

void Menu::handleLongPress() {
  // Toggle the heater on/off
  if (menuActive) {
    exitMenu();
  } else {
    heaterControl.toggleHeater();
  }
}

void Menu::displayMenu() {
  // Only update the display if the menu index has changed
  if (currentMenuIndex != lastMenuIndex) {
    Serial.println("displayMenu");
    lcd.clear();
    lastMenuIndex = currentMenuIndex;

    lcd.print(menuItems[currentMenuIndex].label);

    switch (currentMenuIndex) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getTargetTemperature(), 1);
      lcd.print(F(" C"));
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getCurrentTemperature(), 1);
      lcd.print(F(" C"));
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getHeaterEnabled() ? "ON" : "OFF");
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getHeaterStatus() ? "ON" : "OFF");
      break;
    case 4: {
      uint32_t autoDisableTime = heaterControl.getTimeUntilDisable();
      uint8_t hours, minutes;
      getHoursAndMinutes(autoDisableTime, hours, minutes);

      lcd.setCursor(0, 1);
      lcd.print(F("                "));
      lcd.setCursor(0, 1);
      lcd.print(hours);
      lcd.print(F("h "));
      lcd.print(minutes);
      lcd.print(F("m"));
      break;
    }
    case 5:
      lcd.setCursor(0, 1);
      if (logger.isLoggingEnabled()) {
        String filename = logger.getLogFileName();
        if (filename.length() > 20) {
          // Scroll the filename once if longer than LCD_WIDTH chars
          const int LCD_WIDTH = 20;
          for (size_t i = 0; i < filename.length() - LCD_WIDTH + 1; i++) {
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print(filename.substring(i, i + 20));
            delay(300); // Delay for scrolling effect (adjust timing as needed)
          }
        } else {
          lcd.print(filename);
        }
      } else {
        lcd.print(logger.isLoggingEnabled() ? "ON" : "OFF");
      }
      break;
    }
  }
}

void Menu::selectMenuItem() {
  Serial.println("selectMenuItem");
  if (menuItems[currentMenuIndex].selectHandler != nullptr) {
    (this->*menuItems[currentMenuIndex].selectHandler)();
  }

  // **Fix: Force display update after exiting**
  lastMenuIndex = -1;
  encoder.read();
  displayMenu();
}

void Menu::adjustTargetTemperature() {
  Serial.println("adjustTargetTemperature");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Set Target Temp"));

  float targetTemp = heaterControl.getTargetTemperature();
  lcd.setCursor(0, 1);
  lcd.print(targetTemp, 1); // Print with 1 decimal precision
  lcd.print(F(" C"));

  long encoderPos = encoder.read();
  bool adjusting = true;

  while (adjusting) {
    bounce.update();
    long newEncoderPos = encoder.read();

    // Adjust sensitivity (4 steps per degree)
    if (abs(newEncoderPos - encoderPos) >= 4) {
      targetTemp += (newEncoderPos > encoderPos) ? 1 : -1;
      encoderPos = newEncoderPos;

      // Update the display
      lcd.setCursor(0, 1);
      lcd.print(F("                ")); // Clear the line
      lcd.setCursor(0, 1);
      lcd.print(targetTemp, 1);
      lcd.print(F(" C"));
    }

    if (bounce.rose()) {
      adjusting = false;
    }
  }

  // Update the target temperature
  heaterControl.setTargetTemperature(targetTemp);
}

void Menu::adjustAutoDisable() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Set Auto-Disable"));

  uint32_t autoDisableTime = heaterControl.getTimeUntilDisable();
  long encoderPos = encoder.read();
  bool adjusting = true;

  while (adjusting) {
    bounce.update();
    long newEncoderPos = encoder.read();

    // Adjust sensitivity (4 steps per unit)
    if (abs(newEncoderPos - encoderPos) >= 4) {
      if (newEncoderPos > encoderPos) {
        autoDisableTime +=
            1800000; // Increase by 30 minutes (30 * 60 * 1000 ms)
      } else if (newEncoderPos < encoderPos) {
        if (autoDisableTime >= 1800000) {
          autoDisableTime -= 1800000; // Decrease by 30 minutes
        }
      }
      encoderPos = newEncoderPos;

      // Update the display
      uint8_t hours, minutes;
      getHoursAndMinutes(autoDisableTime, hours, minutes);

      lcd.setCursor(0, 1);
      lcd.print(F("                ")); // Clear the line
      lcd.setCursor(0, 1);
      lcd.print(hours);
      lcd.print(F("h "));
      lcd.print(minutes);
      lcd.print(F("m"));
    }

    if (bounce.rose()) {
      adjusting = false;
    }
  }

  // Update the auto-disable time in the heater control
  heaterControl.setTimeUntilDisable(autoDisableTime);
}

void Menu::toggleLogging() {
  if (logger.toggleLogging() != 0) {
    displayError(logger);
  }
}

void Menu::displayDefaultScreen(float currentTemp, float targetTemp) {
  if (menuActive) {
    return; // Skip updating the default screen when the menu is active
  }

  static float lastCurrentTemp = -999.0;
  static float lastTargetTemp = -999.0;

  // Update the LCD only if the temperature values have changed
  if (currentTemp != lastCurrentTemp || targetTemp != lastTargetTemp) {
    lcd.setCursor(0, 0);
    lcd.print(F("Target: "));
    lcd.print(targetTemp, 1);
    lcd.print(F(" C   ")); // Add spaces to overwrite any previous text

    lcd.setCursor(0, 1);
    lcd.print(F("Current: "));
    lcd.print(currentTemp, 1);
    lcd.print(F(" C   ")); // Add spaces to overwrite any previous text

    lastCurrentTemp = currentTemp;
    lastTargetTemp = targetTemp;
  }
}

// Retrieve and display logger error message
void displayError(Log &logger) {
  Serial.println(logger.getErrorMessage());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(logger.getErrorMessage());
  delay(2000);
}

void getHoursAndMinutes(uint32_t autoDisableTime, uint8_t &hours,
                        uint8_t &minutes) {
  hours = autoDisableTime / 3600000;             // Calculate hours
  minutes = (autoDisableTime % 3600000) / 60000; // Calculate remaining minutes
}
