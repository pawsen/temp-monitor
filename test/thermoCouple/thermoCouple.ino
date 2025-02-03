#include "I2C_LCD.h"
I2C_LCD lcd(0x27);

#include "MAX6675.h"

const uint8_t MAX6675_CS_PINS = 7;
const uint8_t SPI_MISO_PIN = 8;
const uint8_t SPI_SCK_PIN = 9;


// HW SPI:
// | Name                       | pin |
// |----------------------------|-----|
// | CLOCK                      |  13 |
// | MISO (Master in/Slave out) |  12 |
// | MOSI (Master out/Slave in) |  11 |

MAX6675 thermoCouple(MAX6675_CS_PINS, SPI_MISO_PIN , SPI_SCK_PIN);
// MAX6675 thermoCouple(10, &SPI);
// MAX6675 thermoCouple(3, &SPI);


void setup ()
{
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
  lcd.print("Temp");

  SPI.begin();
  thermoCouple.begin();
}


void loop ()
{
  int status = thermoCouple.read();
  if (status != STATUS_OK)
  {
    Serial.println("ERROR!");
  }

  // //  Read the raw Data value from the module
  // uint32_t value = thermoCouple.getRawData();
  // Serial.print("RAW:\t");

  // //  Display the raw data value in BIN format
  // uint32_t mask = 0x80000000;
  // for (int i = 0; i < 32; i++)
  // {
  //   if ((i > 0)  && (i % 4 == 0)) Serial.print("-");
  //   Serial.print((value & mask) ? 1 : 0);
  //   mask >>= 1;
  // }
  // Serial.println();

  Serial.print("TMP:\t");
  float temp = thermoCouple.getTemperature();
  Serial.println(temp, 3);

  lcd.setCursor(8, 0);
  lcd.print(temp);

  delay(1000);
}
