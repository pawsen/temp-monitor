#include <uRTCLib.h>
#include <Arduino.h>

/* NixOS builds packages in a completely reproducible way. As part of this,
 * __DATE__, __TIME__, and __TIMESTAMP__ macros are stripped or replaced with
 * fixed values (usually 1970-01-01) to ensure that builds are deterministic.
 *
 * To set the RTC time to compile time, the variable UNIX_TIMESTAMP is defined
 * in the Makefile and converted to YYMMDDhhmmss
*/

// Set UNIX_TIMESTAMP in the Makefile:
// CPPFLAGS += -DUNIX_TIMESTAMP=$(shell date +%s)
#ifndef UNIX_TIMESTAMP
#define UNIX_TIMESTAMP 0  // Fallback in case it's not defined
#endif

// Define time_t if not already defined
typedef long time_t;
// Global variable for time zone offset in seconds (GMT+1 is 3600 seconds, GMT+2 is 7200, etc.)
int timeZoneOffset = 3600;  // Set default to GMT+1 (3600 seconds)


// URTCLIB_MODEL_DS1307
// URTCLIB_MODEL_DS3231
// URTCLIB_MODEL_DS3232
const byte RTC_MODEL = URTCLIB_MODEL_DS1307;
const byte RTC_ADDRESS = 0x68;

// Create an RTC object
uRTCLib rtc(RTC_ADDRESS, RTC_MODEL); // DS3231 I2C address is 0x68

// Function to print the current RTC time
void printRTCTime() {
  rtc.refresh();
  Serial.print(F("Current RTC time: "));
  Serial.print(rtc.year());
  Serial.print(F("-"));
  Serial.print(rtc.month());
  Serial.print(F("-"));
  Serial.print(rtc.day());
  Serial.print(F(" "));
  Serial.print(rtc.hour());
  Serial.print(F(":"));
  Serial.print(rtc.minute());
  Serial.print(F(":"));
  Serial.println(rtc.second());
}

// Function to get the last Sunday of a given month
int getLastSunday(int month, int year) {
    int dayOfWeek = (5 + (month == 1 || month == 2 ? 31 : 30)) % 7;  // Calculate the first day of the month
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};  // Days in each month
    return daysInMonth[month] - dayOfWeek;  // Find the last Sunday
}

// Function to check if a given year has DST active on a specific date (GMT+1 and GMT+2 DST for Europe)
bool isDST(int year, int month, int day) {
    // DST starts on the last Sunday of March and ends on the last Sunday of October

    // Last Sunday of March and October
    int lastSundayMarch = getLastSunday(3, year);
    int lastSundayOctober = getLastSunday(10, year);

    // Compare if the current date is between the start and end of DST
    bool isInDST = false;
    if (month > 3 && month < 10) {
        isInDST = true;
    } else if (month == 3 && day >= lastSundayMarch) {
        isInDST = true;
    } else if (month == 10 && day <= lastSundayOctober) {
        isInDST = true;
    }

    return isInDST;
}

// Function to convert Unix timestamp to human-readable time
void timestampToDateTime(time_t timestamp, int &year, int &month, int &day, int &hour, int &minute, int &second) {
    // Number of seconds in a day
    const long SECONDS_IN_A_DAY = 86400;

    // Adjust for time zone offset (e.g., GMT+1, GMT+2, etc.)
    timestamp += timeZoneOffset;  // Add the time zone offset in seconds

    // Check for DST and adjust time if necessary
    int yearDst, monthDst, dayDst;
    yearDst = year;
    monthDst = month;
    dayDst = day;

    if (isDST(yearDst, monthDst, dayDst)) {
        timestamp += 3600;  // Add an extra hour for DST (GMT+2)
    }

    // Days since Unix epoch
    long days = timestamp / SECONDS_IN_A_DAY;

    // Calculate year and day of the year
    year = 1970;
    while (true) {
        long daysInYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 366 : 365;  // Leap year check
        if (days < daysInYear) break;
        days -= daysInYear;
        year++;
    }

    // Calculate month and day
    int daysInMonth[] = {31, (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    month = 0;
    while (days >= daysInMonth[month]) {
        days -= daysInMonth[month];
        month++;
    }
    day = days + 1;  // Days in the current month

    // Calculate hour, minute, and second
    long secondsInDay = timestamp % SECONDS_IN_A_DAY;
    hour = secondsInDay / 3600;  // Divide by number of seconds in an hour
    secondsInDay %= 3600;
    minute = secondsInDay / 60;
    second = secondsInDay % 60;
}

// Function to parse the compile date and time from __DATE__ and __TIME__
// macros. These macros are not useful on NixOS
void parseCompileTime(const char *date, const char *time, int &year, int &month, int &day, int &hour, int &minute, int &second) {
  // Parse the date (format: "MMM DD YYYY")
  char monthStr[4];
  sscanf(date, "%s %d %d", monthStr, &day, &year);

  // Convert month string to number
  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  for (month = 0; month < 12; month++) {
    if (strcmp(monthStr, months[month]) == 0) {
      month++; // Months are 1-based (1 = January)
      break;
    }
  }

  // Parse the time (format: "HH:MM:SS")
  sscanf(time, "%d:%d:%d", &hour, &minute, &second);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing RTC...");

#ifdef ARDUINO_ARCH_ESP8266
  URTCLIB_WIRE.begin(0, 2); // D3 and D4 on ESP8266
#else
  URTCLIB_WIRE.begin();
#endif

  // Initialize the RTC
  rtc.refresh();

  Serial.println("Time before updating");
  printRTCTime();

  // Extract compile time from __DATE__ and __TIME__ macros
  const char *compileDate = __DATE__; // Format: "MMM DD YYYY"
  const char *compileTime = __TIME__; // Format: "HH:MM:SS"

  // Parse the compile date and time
  // XXX doesn't work on NixOS
  int year, month, day, hour, minute, second;
  parseCompileTime(compileDate, compileTime, year, month, day, hour, minute, second);

  Serial.println(compileDate);
  Serial.println(compileTime);

  // defined in the Makefile
  Serial.print("Compile time: ");
  Serial.println(UNIX_TIMESTAMP);
  // Convert the Unix timestamp to year, month, day, hour, minute, second
  // int year, month, day, hour, minute, second;
  timestampToDateTime((time_t)UNIX_TIMESTAMP, year, month, day, hour, minute, second);

  Serial.print("Compiled on: ");
  Serial.print(year);
  Serial.print("-");
  Serial.print(month);
  Serial.print("-");
  Serial.print(day);
  Serial.print(" ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);

  // Set the RTC time
  // RTCLib::set(byte second, byte minute, byte hour (0-23:24-hr mode only), byte dayOfWeek (Sun = 1, Sat = 7), byte dayOfMonth (1-12), byte month, byte year)
  // rtc.set(0, 15, 8, 6, 31, 1, 25);
  rtc.set(second, minute, hour, 0, day, month, year % 100); // Year is in YY format (last two digits)
  Serial.println("RTC time set to compile time.");

  // Print the current RTC time
  printRTCTime();
}

void loop() {
  // Nothing to do here
  printRTCTime();
  delay(2000);
}
