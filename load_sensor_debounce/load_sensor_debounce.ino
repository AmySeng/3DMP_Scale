#include "HX711.h"

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight;
float measuredWeight;
float pickedUpThresh = -0.1;
boolean pickedUp = true;

long lastDebounceTime = 0;
long debounceDelay = 1000;

void setup() {

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

void detectChange(){
   totalWeight = scale.get_units(5),1;
  
//   Serial.print("Total Weight: ");
   
   
   if (totalWeight > previousTotalWeight + 3.0 || 
        totalWeight < previousTotalWeight - 3.0){
    lastDebounceTime = millis();
//    Serial.print("different weight: ");
//    Serial.println(totalWeight);
   }

   if (millis() - lastDebounceTime > debounceDelay){
    measuredWeight = totalWeight;
    
    Serial.print("settled weight measured: ");
    Serial.println(measuredWeight);
//    return measuredWeight;
//    Serial.println(measuredWeight);
   
   }

   previousTotalWeight = totalWeight;
//   Serial.print("Previous TotalWeight: ");
//   Serial.println(previousTotalWeight);
}
