#include "HX711.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

#include <HashMap.h>
#include <EEPROM.h>

//--setup SD Logging shield
#define LOG_INTERVAL  1000
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
float measuredWeight, previousMeasuredWeight,
        totalWeight, previousTotalWeight = 0;
byte debounceDelay = 250;
float idWeight;

//const float smallestWeight = 5.4;
byte weightThresh = 7;
boolean movedForReal = false;
boolean pickedUp = false;
boolean noSingleProducts = false;

//byte objectWeight = 0;
//byte objectWeightPlus = 1;
//byte objectWeightMinus = 1;

byte eeprom_address = 0;
byte products_address;
byte singleProducts_address;
const byte singleProducts_count = 5;

byte lookup_address;
//HashType<byte, char*> hashRawArray[58];
//HashMap<byte, char*> lookup = HashMap<byte, char*>(hashRawArray, 58);

 File logfile;

void setup() {
  Serial.begin(9600);

  createProductArrays();

  // DEBUG product arrays
  /****
  byte tmp[13];
  Serial.println();
  EEPROM.get(singleProducts_address, tmp);
  for (byte i = 0; i < 5; i++) {
    Serial.println(tmp[i]);
  }
  Serial.println();
  EEPROM.get(products_address, tmp);
  for (byte i = 0; i < 13; i++) {
    Serial.println(tmp[i]);
  }
  ****/
  
  createLookupTable();

  // DEBUG: print lookup table from EEPROM
  /**
  HashType<byte, char*> tmp_hashArray[16];
  HashMap<byte, char*> tmp_lookup = HashMap<byte, char*>(tmp_hashArray, 16);
  EEPROM.get(lookup_address, tmp_lookup);
  Serial.println(tmp_lookup.getValueOf(6));
  Serial.println(tmp_lookup.getValueOf(10));
  Serial.println(tmp_lookup.getValueOf(2));
  **/


  //---- Logging setup ----//
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

  initializeSD();
  connectToRTC();

  logfile.println("millis,stamp,date,time,item,items,status");

#if ECHO_TO_SERIAL
  Serial.println(F("millis,stamp,date,time,item,items,status"));
#endif //ECHO_TO_SERIAL

  Serial.println(F("2: done setting up logging"));

  //---- Scale setup ---//
  scale.set_scale(441.5f);
  scale.tare();
  Serial.println(F("3: done setting up scale"));
  
}


void loop() {

  digitalWrite(greenLEDpin, HIGH);

  // log milliseconds since starting
  m = millis();


  //the one function to rule them all.
  detectChange();
  delay(50);

}

void createProductArrays() {
  byte products[]  = {
    20, 20, 3, 3, 3, 7, 7, 7, 12, 12, 1, 1, 1
  };
  products_address = eeprom_address;
  EEPROM.put(products_address, products);
  eeprom_address += sizeof(products);

  // create single products array
  byte singleProducts[] = {
    1, 3, 7, 12, 20
  };
  singleProducts_address = eeprom_address;
  EEPROM.put(singleProducts_address, singleProducts);
  eeprom_address += sizeof(singleProducts);
}

void createLookupTable() {
  HashType<byte, char*> hashRawArray[16];
  HashMap<byte, char*> lookup = HashMap<byte, char*>(hashRawArray, 16);
  lookup_address = eeprom_address;
  
  char subset_arr[16][8];
  String subset_tmp;
  byte sum = 0;
  byte product_cnt = 13;
  byte lookup_cnt = 0;
  String middle = ",";
  byte tmp_products[13];
  EEPROM.get(products_address, tmp_products);
  
  for (byte i = 0; i < product_cnt - 1; i++) {
    for (byte j = i + 1; j < product_cnt; j++) {  
      sum = tmp_products[i] + tmp_products[j];
      subset_tmp = tmp_products[i] + middle + tmp_products[j];
      subset_tmp.toCharArray(subset_arr[lookup_cnt], 8);
      if (lookup.getValueOf(sum) == NULL) {
        lookup[lookup_cnt](sum, subset_arr[lookup_cnt]);
        lookup_cnt++;
      }
      else {
        continue;        
      }
    }
  }

  EEPROM.put(lookup_address, lookup);
  eeprom_address += sizeof(lookup);
  lookup.debug();
  Serial.println();
}


void detectChange() {
  totalWeight = scale.get_units(10);
  
  Serial.print("totalWeight: ");
  Serial.println(totalWeight);

  if (abs(totalWeight - previousTotalWeight) > weightThresh) {
    lastDebounceTime = millis();
    movedForReal = true;
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    measuredWeight = totalWeight;
    if (movedForReal) {
      idWeight =  measuredWeight - previousMeasuredWeight;
      Serial.print("idWeight: ");
      Serial.println(idWeight);
      if (idWeight > 0) {
        pickedUp = true;
      }
      else {
        pickedUp = false;
      }

      // keep positive
      idWeight = abs(idWeight);
      checkObjects(idWeight);
      previousMeasuredWeight = measuredWeight;
      movedForReal = false;
    }
  }

  previousTotalWeight = totalWeight;
}

/*
boolean pick(float x) {
  if (x > 0) {
    return true;
  }
  else {
    return false;
  }
}
*/
void checkObjects(float idWeight) {
  float smallestWeight = 20;
  //  Serial.println("in checkObjects");
  // compare to the ratio
  float objectWeight = idWeight / smallestWeight;
  float objectWeightPlus = (idWeight / smallestWeight) - 1;
  float objectWeightMinus = (idWeight / smallestWeight) + 1;
  
  byte singleProducts[singleProducts_count];
  EEPROM.get(singleProducts_address, singleProducts);

  Serial.print(F("Ratio'd Object Weight: "));
  Serial.println(objectWeight);
  // Serial.println(lookup.getValueOf(objectWeight));

  //checking for single products
  for (byte i = 0; i < singleProducts_count; i++) {
    if (abs(singleProducts[i] - objectWeight) < 0.5) {
      if (pickedUp) {
        Serial.print(F("Single Product Picked Up: "));
        Serial.println(singleProducts[i]);
        Serial.println();
        logData( singleProducts[i], "single product", "pick");
        noSingleProducts = false;
      }
      else {
        Serial.print(F("Single Product Put Back: "));
        Serial.println(singleProducts[i]);
        Serial.println();
        logData( singleProducts[i], "single product", "put");
        noSingleProducts = false;
      }
      return;
    }
  }
  //noSingleProducts = true;

  // look up values in table

  HashType<byte, char*> tmp_hashArray[16];
  HashMap<byte, char*> tmp_lookup = HashMap<byte, char*>(tmp_hashArray, 16);
  EEPROM.get(lookup_address, tmp_lookup);
  Serial.println(tmp_lookup.getValueOf(6));
  Serial.println(tmp_lookup.getValueOf(10));
  Serial.println(tmp_lookup.getValueOf(2));
  
  byte byte_weight = byte(objectWeight);
  Serial.print("ROUNDED WEIGHT: ");
  Serial.println(byte_weight);
  char *subset, *subsetMinus, *subsetPlus;


/*
  if (subset != NULL ||
      subsetMinus != NULL ||
      subsetPlus != NULL) {

    if (pickedUp) {
      //      scaleDebug("Picked Up: ", "Object Weight: ", objectWeight);
      Serial.print(F("Two Products Picked Up: "));
      if (subset != NULL) { Serial.println(subset); }
      else if (subsetMinus != NULL) { Serial.println(subsetMinus); }
      else if (subsetPlus != NULL) { Serial.println(subsetPlus); }
      Serial.println();
      //logData( 0, lookup.getValueOf(objectWeight), "pick");
      //logData( singleProducts[i], "single product", "pick");
    }

    else {
      //      scaleDebug("Put Back: ", "Object Weight: ", objectWeight);
      logData( 0, tmp_lookup.getValueOf(objectWeight), "put");
      Serial.print(F("Two Products Put Back: "));
      if (subset != NULL) { Serial.println(subset); }
      else if (subsetMinus != NULL) { Serial.println(subsetMinus); }
      else if (subsetPlus != NULL) { Serial.println(subsetPlus); }
      Serial.println();
    }
    //noSingleProducts = false;
  }
*/
/*
  else if (lookup.getValueOf(objectWeightPlus) != NULL) {

    if (pickedUp) {
      //      scaleDebug("Picked Up: ", "Object Weight - 1: ", objectWeightPlus);
      logData( 0, lookup.getValueOf(objectWeightPlus), "pick");
    }

    else {
      //      scaleDebug("Put Back: ", "Object Weight - 1: ", objectWeightPlus);
      logData( 0, lookup.getValueOf(objectWeightPlus), "put");
    }
    noSingleProducts = false;
  }
  else if (lookup.getValueOf(objectWeightMinus) != NULL) {
    if (pickedUp) {
      //      scaleDebug("Picked Up: ", "Object Weight + 1: ", objectWeightMinus);
      logData( 0, lookup.getValueOf(objectWeightMinus), "pick");
    }

    else {
      //      scaleDebug("Put Back: ", "Object Weight + 1: ", objectWeightMinus);
      logData( 0, lookup.getValueOf(objectWeightMinus), "put");
    }
    noSingleProducts = false;

  }
*/
  else {
    //    Serial.println("unkown weight found");
    //    Serial.println();
    if (pickedUp) {
      Serial.println("unknown weight picked up");
      logData( idWeight, "unknown weight", "pick");
    }
    else {
      Serial.println("unknown weight put back");
      logData( idWeight, "unknown weight", "put");
    }

  }

//  noSingleProducts = true;

  //  Serial.println("check objects");
}


void logData( byte single, char *str, char *stat) {

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
  Serial.print(F(", "));
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
  Serial.println();
#endif //ECHO_TO_SERIAL

  logfile.print(", ");
  logfile.print(single);
  logfile.print(", ");
  logfile.print(str);
  logfile.print(", ");
  logfile.println(stat);

#if ECHO_TO_SERIAL
  logfile.print(F(", "));
  logfile.print(single);
  logfile.print(F(", "));
  logfile.print(str);
  logfile.print(F(", "));
  logfile.println(stat);
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

/*
void scaleDebug( char *state, char *objType, byte obj) {

  Serial.println(state);
  Serial.print(objType);
  Serial.println(lookup.getValueOf(obj));
  Serial.println();

}
*/



