#include "Arduino.h"
#include <math.h>
#include "Enes100.h"
#include <Servo.h>

// ── PIN DEFINITIONS ──
#define ENA 2
#define ENB 3
#define ENC 4
#define END 5
#define IN1 22
#define IN2 24
#define IN3 26
#define IN4 28
#define IN5 23
#define IN6 25
#define IN7 27
#define IN8 29

#define trigPin 30
#define echoPin 31
#define SERVO_PIN 6

// ── SETTINGS ──
const double GOAL_X = 3.70;      
const float sensorThresholdNav2 = 0.15; 
const float sensorThresholdNav1 = 0.05; // Updated to 0.05m for Nav 1
double targetY = 1.0;            
int moveSpeed = 220;  

Servo Hydrogen;

enum State { NAV_1_ALIGN_Y, NAV_2_DRIVE_X, NAV_3_DEPLOY };
State currentState = NAV_1_ALIGN_Y;

// ── MOTOR CONTROL ──────────────────────────────────────────
void setMotors(int speedL, int speedR) {
  analogWrite(ENA, abs(speedL));
  analogWrite(ENB, abs(speedL));
  digitalWrite(IN1, (speedL >= 0) ? LOW : HIGH);
  digitalWrite(IN2, (speedL >= 0) ? HIGH : LOW);
  digitalWrite(IN3, (speedL >= 0) ? HIGH : LOW);
  digitalWrite(IN4, (speedL >= 0) ? LOW : HIGH);

  analogWrite(ENC, abs(speedR));
  analogWrite(END, abs(speedR));
  digitalWrite(IN5, (speedR >= 0) ? HIGH : LOW);
  digitalWrite(IN6, (speedR >= 0) ? LOW : HIGH);
  digitalWrite(IN7, (speedR >= 0) ? LOW : HIGH);
  digitalWrite(IN8, (speedR >= 0) ? HIGH : LOW);
}

void stopAll() { setMotors(0, 0); }

// ── ULTRASONIC LOGIC ───────────────────────────────────────
double Ultra() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 20000);
  float d = (duration * 0.000343) / 2.0;
  return (d <= 0.01) ? 5.0 : d; 
}

// ── PRECISION PIVOT ────────────────────────────────────────
void pivotToAngle(double targetTheta) {
  stopAll();
  delay(500); 
  
  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    
    double curTheta = Enes100.getTheta();
    double error = targetTheta - curTheta;
    while (error > PI) error -= 2 * PI;
    while (error < -PI) error += 2 * PI;

    double absError = abs(error);
    if (absError < 0.05) break; 

    int currentTurnSpeed;
    if (absError > 0.70) currentTurnSpeed = 150;
    else if (absError > 0.26) currentTurnSpeed = 65;
    else if (absError > 0.17) currentTurnSpeed = 50;
    else currentTurnSpeed = 35; 

    if (error > 0) setMotors(-currentTurnSpeed, currentTurnSpeed); 
    else setMotors(currentTurnSpeed, -currentTurnSpeed); 
  }
  stopAll();
  delay(1000); 
}

// ── MAIN LOGIC ─────────────────────────────────────────────

void setup() {
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(ENC, OUTPUT); pinMode(END, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT);

  Enes100.begin("PRIUS", HYDROGEN, 238, 1116, 10, 11);
  
  while (!Enes100.isVisible());
  targetY = (Enes100.getY() > 1.1) ? 0.5 : 1.5;

  Hydrogen.attach(SERVO_PIN);
  Hydrogen.write(170); 
}

void loop() {
  if (!Enes100.isVisible()) { stopAll(); return; }

  double curX = Enes100.getX();
  double curY = Enes100.getY();
  double curTheta = Enes100.getTheta();
  double currentDist = Ultra();

  switch (currentState) {

    case NAV_1_ALIGN_Y:
      Hydrogen.write(160); // Arm position for Nav 1

      // Trigger condition: Target reached OR Obstacle < 0.05m
      if (abs(curY - targetY) < 0.18 || currentDist < sensorThresholdNav1) {
        stopAll();
        Hydrogen.write(170); // RESET ARM IMMEDIATELY
        delay(300);          // Give servo time to move before the 2s pause
        
        Enes100.println("Nav 1 Complete. Resting...");
        delay(2000);           
        
        pivotToAngle(0); 
        currentState = NAV_2_DRIVE_X; 
      } else {
        double angleToLane = (targetY > curY) ? PI/2 : -PI/2;
        if (abs(curTheta - angleToLane) > 0.15) {
          pivotToAngle(angleToLane);
        }
        setMotors(moveSpeed, moveSpeed); 
      }
      break;

    case NAV_2_DRIVE_X:
      if (curX >= GOAL_X) {
        stopAll();
        currentState = NAV_3_DEPLOY;
      } 
      else if (currentDist < sensorThresholdNav2) {
        stopAll();
        setMotors(-160, -160); delay(600); 
        targetY = (targetY < 1.0) ? 1.4 : 0.4; 
        currentState = NAV_1_ALIGN_Y; 
      } 
      else {
        if (abs(curTheta - 0) > 0.12) {
          pivotToAngle(0);
        }
        setMotors(moveSpeed, moveSpeed);
      }
      break;

    case NAV_3_DEPLOY:
      stopAll();
      // Final sweep starting from 170
      for (int pos = 170; pos >= 76; pos -= 1) { 
        Hydrogen.write(pos);              
        delay(30);                        
      }

      setMotors(220, 220); 
      delay(3000);
      
      stopAll();
      Hydrogen.detach(); 
      while(true); 
      break;
  }
}