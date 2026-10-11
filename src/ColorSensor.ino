
// TCS3200 Sensor Pins
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  
  // Set Frequency scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
  Serial.begin(9600);
}

void loop() {
  // Read Red
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  redFrequency = pulseIn(sensorOut, LOW);
  delay(100);

  // Read Green
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  greenFrequency = pulseIn(sensorOut, LOW);
  delay(100);

  // Read Blue
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  blueFrequency = pulseIn(sensorOut, LOW);
  delay(100);

  // Print values for calibration
  Serial.print("R: "); Serial.print(redFrequency);
  Serial.print(" G: "); Serial.print(greenFrequency);
  Serial.print(" B: "); Serial.println(blueFrequency);

  // --- Color Identification Logic ---
  // Note: Lower numbers = Higher intensity
  if (redFrequency < 85 && redFrequency > 25 && greenFrequency < 130 && greenFrequency > 65 && blueFrequency > 75 && blueFrequency < 135) {
    Serial.println(" - YELLOW detected");
  } 
  else if (greenFrequency < 130 && greenFrequency > 85 && redFrequency > 115 && redFrequency < 165 && blueFrequency > 105 && blueFrequency < 145) {
    Serial.println(" - GREEN detected");
  } 
  else if (blueFrequency < 110 && blueFrequency > 70 && redFrequency > 110 && redFrequency < 155 && greenFrequency > 105 && greenFrequency < 150) {
    Serial.println(" - BLUE detected");
  }
  else if (redFrequency < 100 && redFrequency > 25 && greenFrequency > 110 && greenFrequency < 160 && blueFrequency > 105 && blueFrequency < 155) {
    Serial.println(" - RED detected");
  }
  else if (redFrequency < 150 && redFrequency > 80 && greenFrequency < 180 && greenFrequency > 80 && blueFrequency < 160 && blueFrequency > 80) {
    Serial.println(" - WHITE detected");
  }
  else {
    Serial.println(" - COLOR Unknown)");
  }
  delay(500);
}
