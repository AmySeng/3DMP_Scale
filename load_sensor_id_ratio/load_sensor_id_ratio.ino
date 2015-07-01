#include "HX711.h"

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight;
float measuredWeight, previousMeasuredWeight, idWeight;
float paintTube, spotTube;
float pickedUpThresh = -0.1;
float smallestWeight;
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

  //  Serial.println("Detected Change:");
  detectChange();

}

void detectChange() {

  totalWeight = scale.get_units(5), 1;

  //   Serial.print("Total Weight: ");
  if (totalWeight > previousTotalWeight + 3.0 ||
      totalWeight < previousTotalWeight - 3.0) {
    lastDebounceTime = millis();
    pickedUp = true;
    //    Serial.print("different weight: ");
    //    Serial.println(totalWeight);
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    measuredWeight = totalWeight;

    if (pickedUp) {
      idWeight =  measuredWeight - previousMeasuredWeight;
      checkObjects();
      
    byte objectWeight = 0;
    objectWeight = byte(abs(idWeight) / smallestWeight);

    Serial.print("idWeight: ");
    Serial.println(idWeight);
    Serial.print("Object Ratio: ");
    Serial.println(objectWeight);
    Serial.println();
    
      previousMeasuredWeight = measuredWeight;
      pickedUp = false;
    }
  }

  previousTotalWeight = totalWeight;

}

void checkObjects(){
  if (idWeight <= paintTube + 3 && idWeight >= paintTube - 3){
    Serial.println("Paint tube picked up!");
  }
  else if (idWeight >= -paintTube - 3 && idWeight <= -paintTube + 3){
    Serial.println("Paint tube put back!");
  }

  if (idWeight <= spotTube + 3 && idWeight >= spotTube - 3){
    Serial.println("Spot tube picked up!");
  }
   else if (idWeight >= -spotTube - 3 && idWeight <= -spotTube + 3){
    Serial.println("Spot tube put back!");
   }
}

