#include <HashMap.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  byte products[] = {
    1, 1, 1, 3, 3, 3, 7, 7, 15, 15, 31, 31, 31
  };
  byte product_count = 13;

  HashType<byte, char*> hashRawArray[77];
  HashMap<byte, char*> lookup = HashMap<byte, char*>(hashRawArray, 77);

  byte sum = 0;
  byte lookup_cnt = 0;

  char subset[77][10];
  
  for (byte i=0; i < product_count-1; i++) {
    for (byte j=i+1; j < product_count; j++) {
      sum = products[i] + products[j];
      String middle = ", ";
      String subset_tmp = products[i] + middle + products[j];
      subset_tmp.toCharArray(subset[lookup_cnt], 10);
      lookup[lookup_cnt](sum, subset[lookup_cnt]);
      lookup_cnt++;
    }
  }

  lookup.debug();
  
  // FIND Subset
  byte weight = 15;
  Serial.print("Looking for: ");
  Serial.println(weight);
  if (lookup.getValueOf(weight) == NULL) {
    Serial.println("NULL!");
  }
  else {
    Serial.println(lookup.getValueOf(weight));
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
