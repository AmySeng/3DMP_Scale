#include "HX711.h"
#include <HashMap.h>

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight;
float measuredWeight, previousMeasuredWeight, idWeight;
float smallestWeight = 3.5;
float pickedUpThresh = -0.1;
boolean movedForReal = false;
boolean pickedUp = false;
boolean noSingleProducts = false;

byte objectWeight = 0;
byte objectWeightPlus = 1;
byte objectWeightMinus = 2;

long lastDebounceTime = 0;
long debounceDelay = 500;


//float singleProducts[] = {
//  5.6, 16.8, 38.32, 84, 173.6
//};

byte singleProducts[] = {
  1, 3, 7, 9, 20
};

byte  singleProducts_count = 5;

HashType<byte, char*> hashRawArray[78];
HashMap<byte, char*> lookup = HashMap<byte, char*>(hashRawArray, 78);


void setup() {
  Serial.begin(9600);
  byte products[] = {
    20, 20, 1, 3, 3, 3, 7, 7, 7, 9, 9, 1, 1
  };
  byte product_count = 13;

  byte sum = 0;
  byte lookup_cnt = 0;

  char subset[78][10];

  for (byte i = 0; i < product_count - 1; i++) {
    for (byte j = i + 1; j < product_count; j++) {
      sum = products[i] + products[j];
      String middle = ", ";
      String subset_tmp = products[i] + middle + products[j];
      subset_tmp.toCharArray(subset[lookup_cnt], 10);
      lookup[lookup_cnt](sum, subset[lookup_cnt]);
      lookup_cnt++;
    }
  }

  //lookup.debug();

  //---- Scale setup ---//
  scale.read();
  scale.set_scale(2280.f);
  scale.tare();


}

void loop() {

  //the one function to rule them all.
  detectChange();

}


void detectChange() {

  totalWeight = scale.get_units(5), 1;
  //  Serial.println(totalWeight);

  if (totalWeight > previousTotalWeight + 2.0 ||
      totalWeight < previousTotalWeight - 2.0) {

    lastDebounceTime = millis();

    movedForReal = true;
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    measuredWeight = totalWeight;

    if (movedForReal) {

      idWeight =  measuredWeight - previousMeasuredWeight;
      Serial.print("idWeight: ");
      Serial.println(idWeight);

      pickedUp = pick(idWeight);

      // keep positive
      idWeight = abs(idWeight);
      checkObjects();


      previousMeasuredWeight = measuredWeight;

      movedForReal = false;
    }
  }

  previousTotalWeight = totalWeight;
}

boolean pick(float x) {
  if (x > 0) {
    return true;
  }
  else {
    return false;
  }
}

void checkObjects() {

  //  Serial.println("in checkObjects");
  // compare to the ratio
  objectWeight = byte(idWeight / smallestWeight);
  objectWeightPlus = byte((idWeight / smallestWeight) - 1);
  objectWeightMinus = byte((idWeight / smallestWeight) + 1);


  Serial.print("Ratio'd Object Weight: ");
  Serial.println(objectWeight);
  // Serial.println(lookup.getValueOf(objectWeight));

//checking for single products


    for (int i = 0; i < singleProducts_count; i++) {

      if (pickedUp) {
        if (objectWeight == singleProducts[i]) {
          Serial.print("Single Product Picked Up: ");
          Serial.println(singleProducts[i]);
          Serial.println();
          noSingleProducts = false;

        }
        else {
          //noSingleProducts = true;

        }
      }

      else {
        if ( objectWeight == singleProducts[i]) {
          Serial.print("Single Product Put Back: ");
          Serial.println(singleProducts[i]);
          Serial.println();
          noSingleProducts = false;

        }

        else {
          //noSingleProducts = true;

        }
      }

    }


  // look up values in table
  if (noSingleProducts && lookup.getValueOf(objectWeight) != NULL) {

    if (pickedUp) {
      //log picked up
      Serial.println("Picked Up");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
      Serial.println();
    }


    else {
      Serial.println("Put Back");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
      Serial.println();
      //log put back
    }
    noSingleProducts = false;
  }


    else if (noSingleProducts && lookup.getValueOf(objectWeightPlus) != NULL) {
  
      if (pickedUp) {
        Serial.print("Picked Up: ");
        Serial.println(objectWeightPlus);
        Serial.print("Object Weight - 1: ");
        Serial.println(lookup.getValueOf(objectWeightPlus));
        Serial.println();
      }
  
      else {
        Serial.print("Put back: ");
        Serial.println(objectWeightPlus);
        Serial.print("Object Weight - 1: ");
        Serial.println(lookup.getValueOf(objectWeightPlus));
        Serial.println();
      }
      noSingleProducts = false;
    }
    else if ( noSingleProducts && lookup.getValueOf(objectWeightMinus) != NULL) {
      if (pickedUp) {
        Serial.print("Picked Up: ");
        Serial.println(objectWeightMinus);
        Serial.print("Object Weight + 1: ");
        Serial.println(lookup.getValueOf(objectWeightMinus));
        Serial.println();
      }
  
      else {
        Serial.print("Put Back: ");
        Serial.println(objectWeightMinus);
        Serial.print("Object Weight + 1: ");
        Serial.println(lookup.getValueOf(objectWeightMinus));
        Serial.println();
      }
      noSingleProducts = false;
  
    }
  else if (noSingleProducts) {
    Serial.println("unkown weight found");
    Serial.println();
  }

  noSingleProducts = true;

  //  Serial.println("check objects");
}


