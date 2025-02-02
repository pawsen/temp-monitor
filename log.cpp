// log.cpp
#include "log.h"

Log::Log(uint8_t numSensors)
    : numSensors(numSensors), loggingEnabled(false), loggingStartet(false)
#if USE_RTC
      ,
      rtc(RTC_ADDRESS, RTC_MODEL) // Initialize the RTC object here
#endif                            // USE_RTC
{
  data.temperatures = new float[numSensors];
  memset(data.temperatures, 0, numSensors * sizeof(float));
  memset(errorMessage, 0, sizeof(errorMessage));
  memset(logFileName, 0, sizeof(logFileName));
}

Log::~Log() { delete[] data.temperatures; }

int Log::init(uint8_t SD_CS_PIN) {
  _SD_CS_PIN = SD_CS_PIN;

#if USE_RTC
#ifdef ARDUINO_ARCH_ESP8266
  URTCLIB_WIRE.begin(0, 2); // D3 and D4 on ESP8266
#else
  URTCLIB_WIRE.begin();
#endif
#endif // USE_RTC

  // initialize log filename. as either YYMMDD_hhmmss00.bin, if a RTC is
  // present, or TempLog00.bin
  // Increment the last two 00 if the log file gets full and a new is opened,
  // but keep the initial time stamp
#if USE_RTC
  rtc.refresh();
  printRTCTime();
  snprintf(logFileName, sizeof(logFileName),
           "TempLog_%02d%02d%02d_%02d%02d%02d_00.bin", rtc.year(), rtc.month(),
           rtc.day(), rtc.hour(), rtc.minute(), rtc.second());
  // Set callback. Used to set file timestamps
  // FsDateTime::setCallback(SdFat_dateTime);
#else
  logFileName = "TempLog_00.bin";
#endif // USE_RTC

  return startLogging();
}

void Log::errorPrint(const __FlashStringHelper *msg) {
  sd.errorPrint(&Serial, msg);
  // strcpy_P(errorMessage, PSTR("No FAT partition."));
  strcpy_P(errorMessage, (PGM_P)(msg));
}

int Log::enableLogging() {
  // disbale logging if something fails
  loggingEnabled = false;

#if ENABLE_DEDICATED_SPI
  Serial.println(F("\nENABLE_DEDICATED_SPI is set"));
#else
  Serial.println(
      F("\nFor best SD performance edit log.h"
        "use dedicated SPI pins and set ENABLE_DEDICATED_SPI nonzero"));
#endif  // !ENABLE_DEDICATED_SPI

  if (!sd.begin(SD_CONFIG)) {
    strcpy_P(errorMessage, PSTR("SD card init failed"));
    return -1;
  }
  Serial.println(F("SD card initialized."));

  if (!sd.vol()->fatType()) {
    strcpy_P(errorMessage, PSTR("No FAT partition."));
    return -1;
  }
  Serial.println(F("Found FAT partition."));

#ifndef  USE_RTC
  // if a file with logFileName already exists (ie. probably an old logfile),
  // move it
  if (!sd.exists(logFileName)) {
    if (!sd.rename(logFileName, "TempLog_00.bin.bak")){
      Serial.println(F("Error renaming old logfile"));
      return -1;
    }
  }
#endif // USE_RTC

  loggingEnabled = true;
  printSDInfo();
  return 0;
}

bool Log::isLoggingEnabled() {
  return logFile.isOpen();
}

void Log::printSDInfo(){
  float volumesize, freesize;
  uint32_t sectorsPerCluster = sd.vol()->sectorsPerCluster(); // Clusters are collections of sectors
  volumesize = sectorsPerCluster * sd.vol()->clusterCount();  // We'll have a lot of clusters
  volumesize /= 2 * 1000; // in MB. SD card sectors are always 512 bytes (2 sectors are 1 KB)
  freesize =  sectorsPerCluster * (sd.vol()->freeClusterCount());
  freesize /= 2 * 1000;
  Serial.print(F("Size (MB):"));
  Serial.println(volumesize);
  Serial.print(F("Free (MB):"));
  Serial.println(freesize);
}

int Log::startLogging() {
  if (!loggingEnabled) enableLogging();
  if (logFile.isOpen()) {
    Serial.println(F("LogFile is already open. Unessecary call to startLoggign()"));
    return 0;
  }

  if (sd.exists(logFileName)) {
    if (!logFile.open(logFileName, O_RDWR | O_CREAT)) {
      loggingEnabled = false;
      Serial.println(F("Error opening already existing file"));
    }
    Serial.println(F("Logging started to already existing file"));
    return 0;
  } else {
    Serial.println(F("Logging to new file"));
    return openNewLogFile();
  }
}

int Log::stopLogging() {
  if (loggingEnabled && logFile.isOpen()) {
    logFile.truncate();
    logFile.close();
    Serial.println(F("Logging stopped."));
  }
  return 0;
}

int Log::toggleLogging() {
  Serial.println(F("Toggling logging"));
  if (logFile.isOpen()) {
    return stopLogging();
  } else {
    return startLogging();
  }
}

int Log::openNewLogFile() {
  if (!loggingEnabled)
    return 0;
  logFile.close();

  while (sd.exists(logFileName)) {
    char *p = strchr(logFileName, '.');
    if (!p) {
      strcpy_P(errorMessage, PSTR("no dot in filename"));
      return -1;
    }
    while (true) {
      p--;
      if (p < logFileName || *p < '0' || *p > '9') {
        strcpy_P(errorMessage, PSTR("cant create file name"));
        return -1;
      }
      if (p[0] != '9') {
        p[0]++;
        break;
      }
      p[0] = '0';
    }
  }

  if (!logFile.open(logFileName, O_RDWR | O_CREAT)) {
    strcpy_P(errorMessage, PSTR("open Logfile failed"));
    loggingEnabled = false;
    return -1;
  }
  if (!logFile.preAllocate(PREALLOCATE_SIZE_MiB)) {
    strcpy_P(errorMessage, PSTR("preAllocate failed"));
    loggingEnabled = false;
    return -1;
  }

  writeHeader();
  Serial.print(F("Logging to: "));
  Serial.println(logFileName);
  return 0;
}

void Log::writeHeader() {
  if (!loggingEnabled)
    return;

  uint32_t timestamp = getCurrentTimestamp();
  logFile.write("HEADER\n", 7);
  logFile.write(&numSensors, sizeof(numSensors));
  logFile.write(&timestamp, sizeof(timestamp));

  // Write the size of each temperature value (4 for float, 8 for double)
  uint8_t tempSize = sizeof(float);
  logFile.write(&tempSize, sizeof(tempSize));

  logFile.sync();
}

int Log::logData(float *temperatures, bool heaterStatus) {
  if (!loggingEnabled)
    return 0;

  if (isFileSizeExceeded()) {
    logFile.truncate();
    logFile.close();
    int ret = openNewLogFile();
    if (ret != 0)
      return ret;
  }

  memcpy(data.temperatures, temperatures, numSensors * sizeof(float));
  data.heaterStatus = heaterStatus;
  data.timestamp = getCurrentTimestamp();

  // Write the data directly to the SD card
  logFile.write(data.temperatures, sizeof(float) * numSensors);
  logFile.write(&data.heaterStatus, sizeof(bool));
  logFile.write(&data.timestamp, sizeof(uint32_t));

  // Sync the data to the SD card periodically
  // The SdFat library maintains an internal buffer (usually 512 bytes, the size
  // of an SD card block). When you call logFile.write(data), the data is first
  // written to this internal buffer. The library only writes the data to the SD
  // card when:
  // - The internal buffer is full
  // - You explicitly call logFile.sync() or logFile.close(). (and maybe
  // flush()) The reason for calling sync() is to prevent data loss.
  static uint32_t lastSyncTime = 0;
  if (millis() - lastSyncTime > 1000UL * 60) { // Sync every 1s * 60
    logFile.sync();
    lastSyncTime = millis();
  }
  return 0;
}

bool Log::isFileSizeExceeded() {
  return loggingEnabled && logFile.fileSize() > PREALLOCATE_SIZE_MiB;
}

uint32_t Log::getCurrentTimestamp() {
  uint32_t timestamp;
#if USE_RTC
  rtc.refresh(); // Refresh the RTC to get the latest time
  timestamp = encodeTimestamp(rtc.year(), rtc.month(), rtc.day(), rtc.hour(),
                              rtc.minute(), rtc.second());
#else
  timestamp = millis()
#endif // USE_RTC
  return timestamp;
}

#if USE_RTC
// Call back for file timestamps.  Only called for file create and sync().
void Log::SdFat_dateTime(uint16_t *date, uint16_t *time, uint8_t *ms10) {
  // Access the RTC object through a global or static instance
  // static uRTCLib rtc(RTC_ADDRESS, RTC_MODEL);

  // // Return date using FS_DATE macro to format fields.
  // *date = FS_DATE(rtc.year(), rtc.month(), rtc.day());
  // // Return time using FS_TIME macro to format fields.
  // *time = FS_TIME(rtc.hour(), rtc.minute(), rtc.second());
  // // Return low time bits in units of 10 ms.
  // // sets ms10 based on whether the seconds is odd or even
  // *ms10 = rtc.second() & 1 ? 100 : 0;
}

// Function to encode timestamp into uint32_t using bit packing
uint32_t Log::encodeTimestamp(uint8_t year, uint8_t month, uint8_t day,
                              uint8_t hour, uint8_t minute, uint8_t second) {
  /* Encode the timestamp YYMMDDHHMMSS.
  **
  **  uint32_t can store values up to 4294967295. A timestamp as YYMMDDHHMMSS
  **  has a maximum possible value of 991231235959. To make it fit into
  **  uint32_t, bit packing is used.
  **
  **  We can structure the 32-bit timestamp like this:
  **  Year (YY): 6 bits (0–63, covering 2000–2063)
  **  Month (MM): 4 bits (1–12)
  **  Day (DD): 5 bits (1–31)
  **  Hour (HH): 5 bits (0–23)
  **  Minute (MM): 6 bits (0–59)
  **  Second (SS): 6 bits (0–59)
  **  Total: 6 + 4 + 5 + 5 + 6 + 6 = 32 bits
  */
  return (static_cast<uint32_t>(year) << 26) |
         (static_cast<uint32_t>(month) << 22) |
         (static_cast<uint32_t>(day) << 17) |
         (static_cast<uint32_t>(hour) << 12) |
         (static_cast<uint32_t>(minute) << 6) | (static_cast<uint32_t>(second));
}

// Function to decode uint32_t timestamp into individual components
void Log::decodeTimestamp(uint32_t timestamp, uint8_t &year, uint8_t &month,
                          uint8_t &day, uint8_t &hour, uint8_t &minute,
                          uint8_t &second) {
  year = (timestamp >> 26) & 0x3F;
  month = (timestamp >> 22) & 0x0F;
  day = (timestamp >> 17) & 0x1F;
  hour = (timestamp >> 12) & 0x1F;
  minute = (timestamp >> 6) & 0x3F;
  second = timestamp & 0x3F;
}

// Function to print the current RTC time
void Log::printRTCTime() {
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

#endif // USE_RTC
