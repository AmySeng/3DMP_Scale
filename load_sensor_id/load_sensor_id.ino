#include "HX711.h"

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight;
float measuredWeight, previousMeasuredWeight, idWeight;
float paintTube, spotTube;
float pickedUpThresh = -0.1;
boolean pickedUp = false;

long lastDebounceTime = 0;
long debounceDelay = 500;

void setup() {

  paintTube = 36.5;
  spotTube = 4.6;

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
//    Serial.print("measuredWeight: ");
//    Serial.println(measuredWeight);

    if (pickedUp) {
      idWeight =  measuredWeight - previousMeasuredWeight;
      checkObjects();
//      Serial.print("idWeight: ");
//      Serial.println(idWeight);
      previousMeasuredWeight = measuredWeight;
      pickedUp = false;
    }


    //    return measuredWeight;
    //    Serial.println(measuredWeight);

  }

  previousTotalWeight = totalWeight;
  //   Serial.print("Previous TotalWeight: ");
  //   Serial.println(previousTotalWeight);
}

void checkObjects(){
  if (idWeight <= 39.5 && idWeight >= 32.5){
    Serial.println("Paint tube picked up!");
  }
  else if (idWeight >= -39.5 && idWeight <= -32.5){
    Serial.println("Paint tube put back!");
  }

  if (idWeight <= 6.6 && idWeight >= 2.6){
    Serial.println("Spot tube picked up!");
  }
   else if (idWeight >= -6.6 && idWeight <= -2.6){
    Serial.println("Spot tube put back!");
   }
}

