// Define the analog pin we are using to read the voltage
#include "Enes100.h"

const int voltagePin = A1; 

void setup() {
  // Start the serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Give the serial monitor a second to open
  delay(1000);
  Serial.println("Starting Voltage Reader...");
  Enes100.begin("PRIUS", HYDROGEN, 534, 1201, 18, 19);
}

void loop() {
  // 1. Read the raw analog value from the pin (0 to 1023)
  int sensorValue = analogRead(voltagePin);

  // 2. Calculate the exact, precise Voltage
  float exactVoltage = sensorValue * (5.0 / 1023.0);

  // 3. "Floor" the voltage to round it strictly down to the nearest whole number
  float flooredVoltage = floor(exactVoltage);

  // 4. Print the result to the Serial Monitor
  Serial.print("Exact: ");
  Serial.print(exactVoltage);
  Serial.print(" V  |  Floored: ");
  Serial.print(flooredVoltage);
  Serial.println(" V");

  if ((exactVoltage > 0.9) && (exactVoltage < 1.1)) {
    delay(500);
    Enes100.println("Voltage is 1");
    Enes100.mission(exactVoltage, VOLTAGE_1);
  }
  else if((exactVoltage > 1.9) && (exactVoltage < 2.1)) {
    Enes100.println("Voltage is 2");
    Enes100.mission(exactVoltage, VOLTAGE_2);
  }
  else if((exactVoltage > 2.9) && (exactVoltage < 3.1)) {
    Enes100.println("Voltage is 3");
    Enes100.mission(exactVoltage, VOLTAGE_3);
  }
  else if((exactVoltage > 3.9) && (exactVoltage < 4.1)) {
    Enes100.println("Voltage is 4");
    Enes100.mission(exactVoltage, VOLTAGE_4);
  }
  else if(exactVoltage > 4.9) {
    Enes100.println("Voltage is 5");
    Enes100.mission(exactVoltage, VOLTAGE_5);
  }

  // 5. Wait half a second before taking the next reading
  delay(500);
}