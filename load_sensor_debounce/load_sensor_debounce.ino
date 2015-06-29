#include "HX711.h"

HX711 scale(A2, A3);

float totalWeight, previousTotalWeight, measuredWeight;
float pickedUpThresh = -0.1;
boolean pickedUp = true;

long lastDebounceTime = 0;
long debounceDelay = 500;

void setup() {

  Serial.begin(9600);
  //---- Scale setup ---//
  scale.read();
  scale.set_scale(2280.f);
  scale.tare();  

}

void loop() {

   Serial.println(detectChange());

   }

float detectChange(){
   totalWeight = scale.get_units(5),1;
   Serial.println(totalWeight);
   
   if (totalWeight >= previousTotalWeight + 1.0 || 
        totalWeight <= previousTotalWeight - 1.0){
    lastDebounceTime = millis();
    Serial.println("different weight");
   }

   if (millis() - lastDebounceTime > debounceDelay){
    measuredWeight = totalWeight;
    Serial.println("settled weight measured");
    return measuredWeight;
//    Serial.println(measuredWeight);

   previousTotalWeight = totalWeight;
   }
}
