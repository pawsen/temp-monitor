#include "menu.h"

// Constructor with member initializer list for the encoder
Menu::Menu(uint8_t ENCODER_PIN_A, uint8_t ENCODER_PIN_B,
           uint8_t ENCODER_BUTTON_PIN)
    : encoder(ENCODER_PIN_A, ENCODER_PIN_B),
      ENCODER_BUTTON_PIN(ENCODER_BUTTON_PIN) {}

// Initialize button pin and any other hardware
void Menu::init() {
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  Serial.println(F("Menu initialized."));
}

void Menu::update() {
  // Handle menu navigation if the menu is active
  if (menuActive) {
    handleMenuNavigation();
    return;
  }

  unsigned long currentTime = millis(); // Current time for timing logic
  int buttonState = digitalRead(ENCODER_BUTTON_PIN); // Read the button state

  // Check for button press (LOW state means pressed)
  if (buttonState == LOW) {
    if (!buttonPressed) { // Detect new press
      buttonPressed = true;
      lastButtonPress = currentTime; // Record the time of the press
      longPressHandled = false;
    }

    // Check for long press
    if (!longPressHandled &&
        (currentTime - lastButtonPress > 1000)) { // Long press detected
      handleLongPress(); // Handle the long press action
      longPressHandled = true;
    }
  } else {                   // Button released (HIGH state)
    if (buttonPressed) {     // If button was previously pressed
      buttonPressed = false; // Reset button state

      // Only handle a short press if no long press was detected
      if (!longPressHandled) {
        if ((currentTime - lastButtonPress) >= 200) { // Short press detected
          Serial.println(F("Menu is active"));
          menuActive = true; // Activate the menu
          displayMenu();
        }
      }
    }
  }
}

void Menu::handleMenuNavigation() {
  int encoderPos = encoder.read() / 4; // Get the encoder position
  static int lastEncoderPos = 0; // Store the last known position of the encoder

  // Check if the encoder position has changed
  if (encoderPos != lastEncoderPos) {
    if (encoderPos > lastEncoderPos) { // Encoder rotated clockwise
      currentMenuIndex =
          (currentMenuIndex + 1) % 4; // Move to the next menu item
    } else if (encoderPos <
               lastEncoderPos) { // Encoder rotated counterclockwise
      currentMenuIndex =
          (currentMenuIndex - 1 + 4) % 4; // Move to the previous menu item
    }
    lastEncoderPos = encoderPos; // Update the last known encoder position
    displayMenu();               // Refresh the menu display
  }

  // Handle button press for selecting menu items
  static unsigned long lastButtonTime = 0; // Last time the button was pressed
  unsigned long currentTime = millis();    // Current time for debouncing

  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) { // Button is pressed
    if (currentTime - lastButtonTime >
        200) {          // Debounce: Only trigger if 200ms have passed
      selectMenuItem(); // Select the current menu item
      lastButtonTime = currentTime; // Update the last button press time
    }
  }
}

void Menu::handleLongPress() {
  // Toggle the heater on/off
  if (menuActive) {
    Serial.println(F("Exiting menu..."));
    menuActive = false;
    lcd.clear();
  } else {
    heaterControl.toggleHeater();
  }
}

void Menu::displayMenu() {
  static int lastMenuIndex = -1; // Track the last displayed menu index

  // Only update the display if the menu index has changed
  if (currentMenuIndex != lastMenuIndex) {
    lcd.clear(); // Clear the display only when switching menu items
    lastMenuIndex = currentMenuIndex;

    switch (currentMenuIndex) {
    case 0:
      lcd.print(F("Target Temp"));
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getTargetTemperature(), 1);
      lcd.print(F(" C"));
      break;
    case 1:
      lcd.print(F("Current Temp:"));
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getCurrentTemperature(), 1);
      lcd.print(F(" C"));
      break;
    case 2:
      lcd.print(F("Heater: "));
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getHeaterStatus() ? "ON" : "OFF");
      break;
    case 3:
      uint32_t autoDisableTime = heaterControl.getTimeUntilDisable();
      uint32_t hours = autoDisableTime / 3600000; // Calculate hours
      uint32_t minutes =
          (autoDisableTime % 3600000) / 60000; // Calculate remaining minutes

      lcd.print(F("Auto-Disable"));
      lcd.setCursor(0, 1);
      lcd.print(F("                ")); // Clear the line
      lcd.setCursor(0, 1);
      lcd.print(hours);
      lcd.print(F("h "));
      lcd.print(minutes);
      lcd.print(F("m"));
      break;
    }
  }
}

void Menu::selectMenuItem() {
  // do the task. If the menu is "read only", like current temp, do nothing.
  switch (currentMenuIndex) {
  case 0: // Set Target Temp
    adjustTargetTemperature();
    break;
  case 1: // View Temp
    break;
  case 2: // View Heater Status
    break;
  case 3: // Set Auto-Disable Time
    adjustAutoDisable();
    break;
  default:
    // Handle unexpected cases (e.g., invalid menu index)
    Serial.println(F("Invalid menu index!"));
    break;
  }

  Serial.println(F("existing selectMenuItem"));
  // Keep the menu active after selecting a menu item unless it exits to the
  // main loop
  menuActive = false;
  displayMenu();
}

void Menu::adjustTargetTemperature() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Set Target Temp"));

  double targetTemp = heaterControl.getTargetTemperature();
  lcd.setCursor(0, 1);
  lcd.print(targetTemp, 1); // Print with 1 decimal precision
  lcd.print(F(" C"));

  long encoderPos = encoder.read();
  bool adjusting = true;

  // Wait for button release to avoid immediate exit
  while (digitalRead(ENCODER_BUTTON_PIN) == LOW)
    delay(200);

  while (adjusting) {
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

    int buttonState = digitalRead(ENCODER_BUTTON_PIN);

    // Detect new button press
    if (buttonState == LOW) {
      delay(200); // Debounce
      adjusting = false;
    }
  }

  // Update the target temperature
  heaterControl.setTargetTemperature(targetTemp);

  // Return to the main menu display
  displayMenu();
}

void Menu::adjustAutoDisable() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Set Auto-Disable"));

  uint32_t autoDisableTime = heaterControl.getTimeUntilDisable();
  long encoderPos = encoder.read();
  bool adjusting = true;

  // Wait for button release to avoid immediate exit
  while (digitalRead(ENCODER_BUTTON_PIN) == LOW)
    delay(200);

  while (adjusting) {
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
      uint32_t hours = autoDisableTime / 3600000; // Calculate hours
      uint32_t minutes =
          (autoDisableTime % 3600000) / 60000; // Calculate remaining minutes

      lcd.setCursor(0, 1);
      lcd.print(F("                ")); // Clear the line
      lcd.setCursor(0, 1);
      lcd.print(hours);
      lcd.print(F("h "));
      lcd.print(minutes);
      lcd.print(F("m"));
    }

    // Read the button state
    int buttonState = digitalRead(ENCODER_BUTTON_PIN);

    // Detect new button press
    if (buttonState == LOW) {
      delay(200); // Debounce
      adjusting = false;
    }
  }

  // Update the auto-disable time in the heater control
  heaterControl.setTimeUntilDisable(autoDisableTime);

  // Return to the main menu display
  displayMenu();
}

void Menu::displayDefaultScreen(double currentTemp, double targetTemp) {
  if (menuActive) {
    return; // Skip updating the default screen when the menu is active
  }

  static double lastCurrentTemp = -999.0;
  static double lastTargetTemp = -999.0;

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

    // lcd.setCursor(0, 2);
    // lcd.print("Heater: ");
    // lcd.println(heaterStatus ? "ON" : "OFF");
    lastCurrentTemp = currentTemp;
    lastTargetTemp = targetTemp;
  }
}
