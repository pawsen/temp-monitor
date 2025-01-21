#include "menu.h"

Menu menu;

void Menu::init(uint8_t ENCODER_PIN_A, uint8_t ENCODER_PIN_B, uint8_t ENCODER_BUTTON_PIN) {
 this->ENCODER_PIN_A = ENCODER_PIN_A;;
 this->ENCODER_PIN_B = ENCODER_PIN_B;;
 this->ENCODER_BUTTON_PIN = ENCODER_BUTTON_PIN;;
  pinMode(this->ENCODER_BUTTON_PIN, INPUT_PULLUP);
  encoder = Encoder(this->ENCODER_PIN_A, this->ENCODER_PIN_B);
}

void Menu::update() {
  if (menuActive) {
    handleMenuNavigation();
    return;
  }

  unsigned long currentTime = millis();

  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {
    if (currentTime - lastButtonPress > 1000) {  // Long press detected
      handleLongPress();
    } else if (currentTime - lastButtonPress > 200) {  // Short press detected
      menuActive = true;
    }
  }
}

void Menu::handleMenuNavigation() {
  int encoderPos = encoder.read() / 4;  // Adjust rotation sensitivity
  encoder.write(0);  // Reset the encoder position to prevent overflow

  if (encoderPos > 0) {
    currentMenuIndex = (currentMenuIndex + 1) % 4;
  } else if (encoderPos < 0) {
    currentMenuIndex = (currentMenuIndex - 1 + 4) % 4;
  }
  displayMenu();

  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {
    delay(500);  // Debounce delay
    selectMenuItem();
  }
}

void Menu::handleLongPress() {
  heaterControl.toggleHeater();  // Toggle the heater on/off
  menuActive = false;
}

void Menu::selectMenuItem() {
  switch (currentMenuIndex) {
    case 0: // Set Target Temp
      adjustTargetTemperature();
      break;
    case 1: // View Temp
      break; // No action needed for viewing temperature
    case 2: // View Heater Status
      break; // No action needed for heater status
    case 3: // Set Auto-Disable Time
      // heaterControl.setAutoDisableTime(HEATER_TIMEOUT_DEFAULT);
      adjustAutoDisable();
      break;
  }
  menuActive = false;
}

void Menu::displayMenu() {
  lcd.clear();
  switch (currentMenuIndex) {
    case 0:
      lcd.print("Target Temp");
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getTargetTemperature(), 1);
      lcd.print(" C");
      break;
    case 1:
      lcd.print("Current Temp:");
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getCurrentTemperature(), 1);
      lcd.print(" C");
      break;
    case 2:
      lcd.print("Heater: ");
      lcd.setCursor(0, 1);
      lcd.print(heaterControl.getHeaterStatus() ? "ON" : "OFF");
      break;
    case 3:
      uint32_t time = heaterControl.getTimeUntilDisable();
      lcd.print("Set Auto-Disable");
      lcd.setCursor(0, 1);
      lcd.print(time);
      break;
  }
}

void Menu::adjustTargetTemperature() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Target Temp");

    double targetTemp = heaterControl.getTargetTemperature();
    lcd.setCursor(0, 1);
    lcd.print(targetTemp);
    lcd.print(" C");

    int encoderPos = encoder.read();
    while (digitalRead(ENCODER_BUTTON_PIN) == HIGH) {
        if (encoder.read() != encoderPos) {
            targetTemp += (encoder.read() > encoderPos) ? 1 : -1;
            encoderPos = encoder.read();
            lcd.setCursor(0, 1);
            lcd.print(targetTemp);
            lcd.print(" C");
        }
    }
    // Update the target temperature
    heaterControl.setTargetTemperature(targetTemp);
    displayMenu();
}

void Menu::adjustAutoDisable(){
  Serial.println("adjust auto disable");
}
