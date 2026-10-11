// MOTOR PINS (Based on your orientation)
int motorFL_1 = 22; int motorFL_2 = 24; int motorFL_speed = 2; 
int motorBL_1 = 26; int motorBL_2 = 28; int motorBL_speed = 3; 

int motorFR_1 = 23; int motorFR_2 = 25; int motorFR_speed = 4; 
int motorBR_1 = 27; int motorBR_2 = 29; int motorBR_speed = 5; 

void setup() {
  pinMode(motorFL_1, OUTPUT); pinMode(motorFL_2, OUTPUT); pinMode(motorFL_speed, OUTPUT);
  pinMode(motorBL_1, OUTPUT); pinMode(motorBL_2, OUTPUT); pinMode(motorBL_speed, OUTPUT);
  pinMode(motorFR_1, OUTPUT); pinMode(motorFR_2, OUTPUT); pinMode(motorFR_speed, OUTPUT);
  pinMode(motorBR_1, OUTPUT); pinMode(motorBR_2, OUTPUT); pinMode(motorBR_speed, OUTPUT);
}

// Direction helper to keep the main code clean
void drive(int p1, int p2, int spdPin, int speed, bool forward) {
  analogWrite(spdPin, speed);
  digitalWrite(p1, forward ? HIGH : LOW);
  digitalWrite(p2, forward ? LOW : HIGH);
}

void strafeLeft(int speed) {
  // To move RIGHT:
  drive(motorFL_1, motorFL_2, motorFL_speed, speed, true); 
  drive(motorBR_1, motorBR_2, motorBR_speed, speed, true);  
  
  drive(motorBL_1, motorBL_2, motorBL_speed, speed, true); 
  drive(motorFR_1, motorFR_2, motorFR_speed, speed, true); 
}

void strafeRight(int speed) {
  // To move LEFT:
  drive(motorFL_1, motorFL_2, motorFL_speed, speed, false); 
  drive(motorBR_1, motorBR_2, motorBR_speed, speed, false); 
  
  drive(motorBL_1, motorBL_2, motorBL_speed, speed, false);  
  drive(motorFR_1, motorFR_2, motorFR_speed, speed, false);  
}

void stopAll() {
  analogWrite(motorFL_speed, 0); analogWrite(motorBL_speed, 0);
  analogWrite(motorFR_speed, 0); analogWrite(motorBR_speed, 0);
}

void loop() {
  strafeRight(200);
  delay(2000);

}