#include "HX711.h"
#include <HashMap.h>

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight;
float measuredWeight, previousMeasuredWeight, idWeight;
float paintTube, spotTube, smallestWeight;
float pickedUpThresh = -0.1;
byte objectWeight;
boolean movedForReal = false;
boolean pickedUp = false;

long lastDebounceTime = 0;
long debounceDelay = 250;

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

boolean pick(float x){
  float picked;
  picked = x + abs(x);
  if (picked > 0){
    return true;
  }
}

void checkObjects() {

  byte products[] = {
    1, 1, 1, 3, 3, 3, 7, 7, 15, 15, 31, 31, 31
  };
  byte product_count = 13;

  HashType<byte, char*> hashRawArray[77];
  HashMap<byte, char*> lookup = HashMap<byte, char*>(hashRawArray, 77);

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

  objectWeight = byte(abs(idWeight / smallestWeight));

  if (lookup.getValueOf(objectWeight) != NULL) {
    //log picked up
    if (!pickedUp){
    //log put back  
    }
  }
  else if (lookup.getValueOf(objectWeight + 1) != NULL) {
   
    if (!pickedUp){
      
    }
  }
  else if (lookup.getValueOf(objectWeight - 1) != NULL) {
   
    if (!pickedUp){
      
    }

  }
  else {

    if (idWeight <= paintTube + 3 && idWeight >= paintTube - 3) {
      Serial.println("Paint tube picked up!");
    }
    else if (idWeight >= -paintTube - 3 && idWeight <= -paintTube + 3) {
      Serial.println("Paint tube put back!");
    }

    if (idWeight <= spotTube + 3 && idWeight >= spotTube - 3) {
      Serial.println("Spot tube picked up!");
    }
    else if (idWeight >= -spotTube - 3 && idWeight <= -spotTube + 3) {
      Serial.println("Spot tube put back!");
    }
    else {
      //something else has SERIOUSLY GONE WRONG
    }

  }


}

