const int trigPin = 9;
const int echoPin = 10;

long duration;
float distanceMM;

void setup() {
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);  
  Serial.begin(9600);       
  Serial.println("--- Ultrasonic Sensor Calibration Test ---");
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceMM = (duration * 0.343) / 2;

  Serial.print("Current Reading: ");
  Serial.print(distanceMM);
  Serial.print(" mm");

  if (distanceMM > 0 && distanceMM < 150) {
    Serial.println(" -> [CLOSE TO NODE - STOP MOTORS]");
  } else {
    Serial.println(" -> [CONTINUE]");
  }

  delay(500);
}