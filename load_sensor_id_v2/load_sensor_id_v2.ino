#include "HX711.h"
#include <HashMap.h>

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight;
float measuredWeight, previousMeasuredWeight, idWeight;
float paintTube, spotTube, smallestWeight;
float pickedUpThresh = -0.1;
boolean movedForReal = false;
boolean pickedUp = false;

long lastDebounceTime = 0;
long debounceDelay = 250;

byte products[] = {
  1, 1, 1, 3, 3, 3, 7, 7, 15, 15, 31, 31, 31
};
byte product_count = 13;

float singleProducts[] = {
  4.6, 36.5
};

float  singleProducts_count = 2;

HashType<byte, char*> hashRawArray[77];
HashMap<byte, char*> lookup = HashMap<byte, char*>(hashRawArray, 77);

void setup() {

  paintTube = 36.5;
  spotTube = 4.6;
  smallestWeight = 4.6;

  Serial.begin(9600);

  //---- Scale setup ---//
  scale.read();
  scale.set_scale(2280.f);
  scale.tare();


}

void loop() {

  detectChange();

}

void detectChange() {

  totalWeight = scale.get_units(5), 1;

  if (totalWeight > previousTotalWeight + 3.0 ||
      totalWeight < previousTotalWeight - 3.0) {

    lastDebounceTime = millis();

    movedForReal = true;
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    measuredWeight = totalWeight;

    if (movedForReal) {
      idWeight =  measuredWeight - previousMeasuredWeight;

      pickedUp = pick(idWeight);
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
}

void checkObjects() {

  byte sum = 0;
  byte lookup_cnt = 0;
  byte objectWeight = 0;

  char subset[77][10];

  for (int i = 0; i < product_count - 1; i++) {
    for (int j = i + 1; j < product_count; j++) {
      sum = products[i] + products[j];
      String middle = ", ";
      String subset_tmp = products[i] + middle + products[j];
      subset_tmp.toCharArray(subset[lookup_cnt], 10);
      lookup[lookup_cnt](sum, subset[lookup_cnt]);
      lookup_cnt++;
    }
  }


// compare to the ratio
  objectWeight = byte(abs(idWeight) / smallestWeight);

// look up values in table
  if (lookup.getValueOf(objectWeight) != NULL) {
    //log picked up
    if (!pickedUp) {
      //log put back
    }
  }
  else if (lookup.getValueOf(objectWeight + 1) != NULL) {

    if (!pickedUp) {

    }
  }
  else if (lookup.getValueOf(objectWeight - 1) != NULL) {

    if (!pickedUp) {

    }

  }
  else {

    for (int i = 0; i < singleProducts_count; i++) {

      if (idWeight <= singleProducts[i] + 3 && idWeight >= singleProducts[i] - 3) {

      }
      else if (idWeight >= -singleProducts[i] - 3 && idWeight <= -singleProducts[i] + 3) {

      }
      else {
        //something else has SERIOUSLY GONE WRONG
      }

    }

  }

}

