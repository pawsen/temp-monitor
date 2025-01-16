
#include "I2C_LCD.h"
#include "MAX6675.h"
#include <PID_v1.h>

I2C_LCD lcd(0x27);

const int ThermoCouplesNum = 1;
MAX6675 ThermoCouples[ThermoCouplesNum] = {
    // selectPin, dataPin, clockPin
    MAX6675(3, &SPI), //  HW SPI
};
const double TEMP_CORRECTION_OFFSET = 0;
const double TEMP_ALARM_MIN = 0;
const double TEMP_ALARM_MAX = 50;

// PID
// define Variables we'll be connecting to
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
const double Kp = 2, Ki = 5, Kd = 1;
const double K = 1; // K is fudge factor for Output Gain or Thermal Resistance

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
const int PIN_INPUT = 0;
const int PIN_OUTPUT = 3;

// PWM on 2 pins (+/-) using Timer 2 (16 bits)
const int PWM_Pin_plus = 11;
const int PWM_Pin_minus = 3;

/* PID limits should reflect actual physical capability. */
const int PID_MIN = -255;
const int PID_MAX = 255;

const int PIN_LED = 8;

uint32_t start, stop;




void PWM_OUT(int signal) {
  if (signal > 0) {
    analogWrite(PWM_Pin_plus, signal);
    analogWrite(PWM_Pin_minus, 0);
  } else {
    analogWrite(PWM_Pin_plus, 0);
    analogWrite(PWM_Pin_minus, 0 - signal);
  }
}

void temp_alarm(double alarm_temp) {
  PWM_OUT(0);                   // turn off forcing functions
  pinMode(PWM_Pin_plus, INPUT); // disable output pins
  pinMode(PWM_Pin_minus, INPUT);
  Serial.print(alarm_temp);
  Serial.print("\nTemperature Alarm - Program End.\n");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(alarm_temp);
  lcd.setCursor(0, 1);
  lcd.print("TEMP ALARM - END");
  delay(100);
  exit(1);
}

void set_temp_pid(MAX6675 ThermoCouple, double target) {

  // myPID references Setpoint, see the constructor
  Setpoint = target;

  float temp = ThermoCouple.getTemperature() - TEMP_CORRECTION_OFFSET;
  Serial.println(temp);
  // Input = temp;  // uncomment to use for live operation
  Input = 20;
  if (Input <= TEMP_ALARM_MIN || Input >= TEMP_ALARM_MAX) {
    temp_alarm(Input);
  }

  myPID.Compute();

  if (Output <= PID_MIN || Output >= PID_MAX) {
    Serial.println("PID error");
  }
  Input = Input + K * Output;
  PWM_OUT((int)K * Output); // update the control outputs
  // stepN++;
  // delay(PIDSAMPLETIME_MS / CLK_DIV);

  return;
}

void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("MAX6675_LIB_VERSION: ");
  Serial.println(MAX6675_LIB_VERSION);
  Serial.println();

  Serial.print("I2C_LCD_LIB_VERSION: ");
  Serial.println(I2C_LCD_LIB_VERSION);
  Serial.println();

  delay(250);

  Wire.begin();
  lcd.begin(16, 2);
  //  display fixed text once
  lcd.setCursor(0, 0);
  lcd.print("temp");

  SPI.begin();
  for (int i = 0; i < ThermoCouplesNum; i++) {
    ThermoCouples[i].begin();
    ThermoCouples[i].setSPIspeed(4000000);
  }

  pinMode(PIN_LED, OUTPUT);

  // initialize the variables we're linked to
  // Input = analogRead(PIN_INPUT);
  // Setpoint = 100;

  // turn the PID on
  myPID.SetMode(AUTOMATIC);
}

int ledState = LOW;
void loop() {

  for (int THCnumber = 0; THCnumber < ThermoCouplesNum; THCnumber++) {
    start = micros();
    int status = ThermoCouples[THCnumber].read();
    stop = micros();
    if (status != STATUS_OK) {
      Serial.println("ERROR!");
    }

    //  Read the raw Data value from the module
    uint32_t value = ThermoCouples[THCnumber].getRawData();
    Serial.print("RAW:\t");

    //  Display the raw data value in BIN format
    uint32_t mask = 0x80000000;
    for (int i = 0; i < 32; i++) {
      if ((i > 0) && (i % 4 == 0))
        Serial.print("-");
      Serial.print((value & mask) ? 1 : 0);
      mask >>= 1;
    }
    Serial.println();

    float temp = ThermoCouples[THCnumber].getTemperature();

    Serial.print("millis: ");
    Serial.print(stop - start);
    Serial.print("\tID: ");
    Serial.print(THCnumber);
    Serial.print("\tstatus: ");
    Serial.print(status);
    Serial.print("\ttemp: ");
    Serial.print(temp);
    Serial.print("\tus: ");
    Serial.println(stop - start);

    delay(1000); //  time to flush all Serial stuff
    lcd.setCursor(8, 0);
    lcd.print(temp);

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    digitalWrite(PIN_LED, ledState);
  }

  float target = 100;
  // use first thermocouple
  set_temp_pid(ThermoCouples[0], target);
}
