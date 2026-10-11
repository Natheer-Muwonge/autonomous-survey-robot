#include <Servo.h>

  Servo Hydrogen;
  Servo Oxygen;

void setup() {
Hydrogen.attach(8);
Oxygen.attach(7);

}

void loop() {
  Hydrogen.write(0);
  Oxygen.write(180);
  delay(10000);
  Hydrogen.write(90);
  Oxygen.write(90);
  delay(10000);

}
