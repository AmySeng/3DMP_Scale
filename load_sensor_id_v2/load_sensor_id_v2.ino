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
long debounceDelay = 500;

byte products[] = {
  1, 1, 1, 3, 3, 3, 7, 7, 15, 15, 15, 20, 20
};
byte product_count = 13;

float singleProducts[] = {
  5.6, 16.8, 38.32, 84, 173.6
};

float  singleProducts_count = 5;

HashType<byte, char*> hashRawArray[77];
HashMap<byte, char*> lookup = HashMap<byte, char*>(hashRawArray, 77);



void setup() {

  smallestWeight = 5.4;

  Serial.begin(9600);

  byte sum = 0;
  byte lookup_cnt = 0;

  char subset[77][10];

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

  if (totalWeight > previousTotalWeight + 3.0 ||
      totalWeight < previousTotalWeight - 3.0) {

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

  byte objectWeight = 0;
  //  Serial.println("in checkObjects");


  // compare to the ratio
  objectWeight = byte(idWeight / smallestWeight);

  Serial.print("Ratio'd Object Weight: ");
  Serial.println(objectWeight);

  // look up values in table
  if (lookup.getValueOf(objectWeight) != NULL) {

    if (pickedUp) {
      //log picked up
      Serial.println("Picked Up");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
    }


    else {
      Serial.println("Put Back");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
      //log put back
    }
  }


  else if (lookup.getValueOf(objectWeight + 1) != NULL) {

    if (pickedUp) {
      Serial.println("Picked Up");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
    }

    else {
      Serial.println("Put back");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
    }
  }
  else if (lookup.getValueOf(objectWeight - 1) != NULL) {
    if (pickedUp) {
      Serial.println("Picked Up");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
    }

    else {
      Serial.println("Put Back");
      Serial.print("Object Weight: ");
      Serial.println(lookup.getValueOf(objectWeight));
    }

  }
  else {

    Serial.println("inside single product loop");

boolean noSingleProducts = false;

    for (int i = 0; i < singleProducts_count; i++) {

      //      Serial.print("Single Product: ");
      //      Serial.println(singleProducts[i]);

      if (pickedUp) {
        if (idWeight <= singleProducts[i] + 3 && idWeight >= singleProducts[i] - 3) {
          Serial.println("found single product picked up");
          noSingleProducts = false;
        }
        else {
          noSingleProducts = true;
        }
      }

      else {
        if ( idWeight <= singleProducts[i] + 3 && idWeight >= singleProducts[i] - 3) {
          Serial.println("found single product put back");
          noSingleProducts = false;
        }
        
        else {
          noSingleProducts = true;
        }
      }
    }

    if (pickedUp && noSingleProducts){
      Serial.println("didn't find a single product picked up, things are weird");
      noSingleProducts = false;
    }
    else{
      Serial.println("didn't find a single product put back, things are weird");
      noSingleProducts = false;
    }

  }

  //  Serial.println("check objects");
}

