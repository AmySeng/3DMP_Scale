#include "HX711.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <HashMap.h>

//--setup SD Logging shield
#define SYNC_INTERVAL 1000
uint32_t syncTime = 0;
uint32_t m = 0;

#define ECHO_TO_SERIAL   1

#define redLEDpin 2
#define greenLEDpin 3

RTC_DS1307 RTC;

const int chipSelect = 10;

//--setup Scale
HX711 scale(A2, A3);
long lastDebounceTime = 0;
float previousMeasuredWeight, previousTotalWeight = 0;
byte debounceDelay = 250;

boolean movedForReal = false;
boolean pickedUp = false;

byte weightThresh = 9;

File logfile;

HashType<int, char*> single_lookup_RawArray[5];
HashMap<int, char*> single_lookup = HashMap<int, char*>(single_lookup_RawArray, 5);

HashType<int, char*> hashRawArray[15];
HashMap<int, char*> lookup = HashMap<int, char*>(hashRawArray, 15);

void setup() {
  Serial.begin(9600);

  single_lookup[0](20, "spot healer");
  single_lookup[1](60, "nail stick");
  single_lookup[2](140, "lotion");
  single_lookup[3](240, "salve");
  single_lookup[4](400, "soap");

  lookup[0](40, "spot healer,spot healer");
  lookup[1](80, "spot healer,nail stick");
  lookup[5](120, "nail stick,nail stick");
  lookup[2](160, "spot healer,lotion");
  lookup[6](200, "nail stick,lotion");
  lookup[3](260, "spot healer,salve");
  lookup[9](280, "lotion,lotion");
  lookup[7](300, "nail stick,salve");
  lookup[10](380, "lotion,salve");
  lookup[4](420, "spot healer,soap");
  lookup[8](460, "nail stick,soap");
  lookup[12](480, "salve,salve");
  lookup[11](540, "lotion,soap");
  lookup[13](640, "salve,soap");
  lookup[14](800, "soap,soap");

  //---- Logging setup ----//
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  
  initializeSD();  
  connectToRTC();

  logfile.println(F("millis, date, time, weight, items, status"));

  //---- Scale setup ---//
  scale.set_scale(428.92f);
  scale.tare();
  Serial.println(F("Ready to go..."));
  Serial.println();
  #if ECHO_TO_SERIAL
    Serial.println(F("millis, date, time, weight, items, status"));
  #endif //ECHO_TO_SERIAL
  
  return;
}


void loop() {
  digitalWrite(greenLEDpin, HIGH);
  // log milliseconds since starting
  m = millis();

  //the one function to rule them all.
  detectChange();
  delay(50);
}

void detectChange() {
  float idWeight;
  float totalWeight = scale.get_units(5);
  float measuredWeight = 0;
  
  if (abs(totalWeight - previousTotalWeight) > weightThresh) {
    lastDebounceTime = millis();
    movedForReal = true;
  }
  if (millis() - lastDebounceTime > debounceDelay) {
    measuredWeight = totalWeight;
    if (movedForReal) {
      idWeight =  measuredWeight - previousMeasuredWeight;
      if (idWeight > 0) {
        pickedUp = true;
      }
      else {
        pickedUp = false;
      }
      checkObjects(abs(idWeight));
      previousMeasuredWeight = measuredWeight;
      movedForReal = false;
    }    
  }
  
  previousTotalWeight = totalWeight;
  return;
}

void checkObjects(float idWeight) {
  int i, intWeight;
  
  intWeight= int(idWeight);
  
  //CHECK FOR SINGLE PRODUCTS 
  for (i = intWeight - weightThresh; i <= intWeight + weightThresh; i++) {
    if (single_lookup.getValueOf(i) != NULL) {
      //Serial.println(i);
      if (pickedUp) {
        logData(i, single_lookup.getValueOf(i), "pick");
      }
      else {
        logData(i, single_lookup.getValueOf(i), "put");
      }
      return;  
    }
  }
  
   for (i = intWeight - weightThresh; i < intWeight + weightThresh; i++) {
    if (lookup.getValueOf(i) != NULL) {
      //Serial.println(i);
      String subset = lookup.getValueOf(i);
      byte index = subset.indexOf(",");
      if (pickedUp) {
        logData(i, subset.substring(0, index), "pick");
        logData(i, subset.substring(index+1), "pick");
      }
      else {
        logData(i, subset.substring(0, index), "put");
        logData(i, subset.substring(index+1), "put");
      }
      return;
    }
   }
  //Serial.println(intWeight);
  if (pickedUp) {
    logData(intWeight, "unknown weight", "pick");
  }
  else {
    logData(intWeight, "unknown weight", "put");
  }
  return;
}

void logData( int weight, String prod, char *stat) {

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

  //log activity
  logfile.print(", ");
  logfile.print(weight);
  logfile.print(", ");
  logfile.print(prod);
  logfile.print(", ");
  logfile.print(stat);
  logfile.println();

#if ECHO_TO_SERIAL
  Serial.print(now.year(), DEC);
  Serial.print(F("/"));
  Serial.print(now.month(), DEC);
  Serial.print(F("/"));
  Serial.print(now.day(), DEC);
  Serial.print(F(", "));

  Serial.print(now.hour(), DEC);
  Serial.print(F(":"));
  Serial.print(now.minute(), DEC);
  Serial.print(F(":"));
  Serial.print(now.second(), DEC);

  Serial.print(F(", "));
  Serial.print(weight);
  Serial.print(F(", "));
  Serial.print(prod);
  Serial.print(F(", "));
  Serial.print(stat);
  Serial.println();
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
  Serial.print(F("error: "));
  Serial.println(str);

  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while (1);
}

void initializeSD() {

  // initialize the SD card
  Serial.print(F("Initializing SD card..."));
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println(F("card initialized."));

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

  Serial.print(F("Logging to: "));
  Serial.println(filename);
}

void connectToRTC() {
  // connect to RTC
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println(F("RTC failed"));
   
    #if ECHO_TO_SERIAL
      Serial.println(F("RTC failed"));
    #endif  //ECHO_TO_SERIAL
    
  }
}

