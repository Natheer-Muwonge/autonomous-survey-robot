// Define the analog pin we are using to read the voltage
const int voltagePin = A1; 

void setup() {
  // Start the serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Give the serial monitor a second to open
  delay(1000);
  Serial.println("Starting Voltage Reader...");
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

  // 5. Wait half a second before taking the next reading
  delay(500);
}