#include "Enes100.h"
#include <math.h>

//MOTOR PINS
//LEFT MOTORS (Driver 1)
int motorFL_1 = 22; int motorFL_2 = 24; int motorFL_speed = 2; //Front Left
int motorBL_1 = 26; int motorBL_2 = 28; int motorBL_speed = 3; //Back Left

//RIGHT MOTORS (Driver 2)
int motorFR_1 = 23; int motorFR_2 = 25; int motorFR_speed = 4; //Front Right
int motorBR_1 = 27; int motorBR_2 = 29; int motorBR_speed = 5; //Back Right

//SENSOR PINS
const int trigPin = 30;
const int echoPin = 31;

//MISSION COORDINATES
double targetX = 1.2; 
double targetY = 0.8;

void setup() {
  //Initialize all direction and speed pins
  int allPins[] = {22, 24, 26, 28, 2, 3, 23, 25, 27, 29, 4, 5};
  for(int i = 0; i < 12; i++) pinMode(allPins[i], OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Team Name, Mission Type, Marker ID, Room Number, Wifi Module TX Pin, Wifi Module RX Pin
  Enes100.begin("PRIUS", HYDROGEN, 18, 19, 205, 1116, 3, 2); 
}

void loop() {
  if (Enes100.updateLocation()) {
    long distMM = getDistance(); //Real-time ultrasonic distance

    //Calculate heading using Vision System
    double dX = targetX - Enes100.getX();
    double dY = targetY - Enes100.getY();
    double targetAngle = atan2(dY, dX);
    
    double angleError = targetAngle - Enes100.getTheta();
    while (angleError > M_PI) angleError -= 2 * M_PI;
    while (angleError < -M_PI) angleError += 2 * M_PI;

    //NAVIGATION LOGIC
    if (distMM > 0 && distMM <= 170) { 
      //ACTION: STOP (Reached the 150mm mark + 20mm braking buffer)
      stopMotors();
      Enes100.print("Target Reached! 150mm verified.");
    } 
    else if (abs(angleError) > 0.3) {
      //ACTION: Turn to face target
      if (angleError > 0) turnLeft(150);
      else                turnRight(150);
    } 
    else {
      //ACTION: Drive forward
      driveForward(180); 
    }
  }
}

//ULTRASONIC MATH
long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999;
  return (duration * 0.343) / 2.0;
}

//MOTOR MOVEMENT HELPERS
void setMotor(int p1, int p2, int sPin, int speed) {
  digitalWrite(p1, speed >= 0 ? HIGH : LOW);
  digitalWrite(p2, speed >= 0 ? LOW : HIGH);
  analogWrite(sPin, abs(speed));
}

void driveForward(int s) {
  setMotor(motorFL_1, motorFL_2, motorFL_speed, s);
  setMotor(motorBL_1, motorBL_2, motorBL_speed, s);
  setMotor(motorFR_1, motorFR_2, motorFR_speed, s);
  setMotor(motorBR_1, motorBR_2, motorBR_speed, s);
}

void turnLeft(int s) {
  setMotor(motorFL_1, motorFL_2, motorFL_speed, -s);
  setMotor(motorBL_1, motorBL_2, motorBL_speed, -s);
  setMotor(motorFR_1, motorFR_2, motorFR_speed, s);
  setMotor(motorBR_1, motorBR_2, motorBR_speed, s);
}

void turnRight(int s) {
  setMotor(motorFL_1, motorFL_2, motorFL_speed, s);
  setMotor(motorBL_1, motorBL_2, motorBL_speed, s);
  setMotor(motorFR_1, motorFR_2, motorFR_speed, -s);
  setMotor(motorBR_1, motorBR_2, motorBR_speed, -s);
}

void stopMotors() { driveForward(0); }