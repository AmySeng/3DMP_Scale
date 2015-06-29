#include "HX711.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

#define LOG_INTERVAL  1000
#define SYNC_INTERVAL 1000
uint32_t syncTime = 0;
uint32_t m = 0;

#define ECHO_TO_SERIAL   1

#define redLEDpin 2
#define greenLEDpin 3

RTC_DS1307 RTC;

const int chipSelect = 10;

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight, measuredWeight;
float pickedUpThresh = -0.1;
boolean pickedUp = true;

long lastDebounceTime = 0;
long debounceDelay = 500;

File logfile;

void setup() {
  Serial.begin(38400);

  //---- Logging setup ----//
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

  initializeSD();
  connectToRTC();

  logfile.println("millis,stamp,date,time,sensor name,status");
#if ECHO_TO_SERIAL
  Serial.println("millis,stamp,date,time,sensor name,status");
#endif //ECHO_TO_SERIAL
  // hello!

  //---- Scale setup ---//
  scale.read();
  scale.set_scale(2280.f);
  scale.tare();

}

void loop() {

 // detectChange();

}

float detectChange() {
  totalWeight = scale.get_units(5), 1;
  Serial.println(totalWeight);

  if (totalWeight >= previousTotalWeight + 1.0 ||
      totalWeight <= previousTotalWeight - 1.0) {
    lastDebounceTime = millis();
    Serial.println("different weight");
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    measuredWeight = totalWeight;
    Serial.println("settled weight measured");
    return measuredWeight;
  }

  previousTotalWeight = totalWeight;
}

void logData(char *str, char *stat) {

  DateTime now;

  logfile.print(m);           // milliseconds since start
  logfile.print(", ");

#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");
#endif

  // fetch the time
  now = RTC.now();

  // log date
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(", ");

  //log time
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);


#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(", ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
#endif //ECHO_TO_SERIAL

  logfile.print(", ");
  logfile.print(str);
  logfile.print(", ");
  logfile.println(stat);

#if ECHO_TO_SERIAL
  Serial.print(", ");
  Serial.print(str);
  Serial.print(", ");
  Serial.println(stat);
#endif ECHO_TO_SERIAL

  digitalWrite(greenLEDpin, LOW);

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();

  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);
}

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while (1);
}

void initializeSD() {

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");

  // create a new file
  char filename[] = "3DMP00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = i / 10 + '0';
    filename[5] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);
}

void connectToRTC() {
  // connect to RTC
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
}
