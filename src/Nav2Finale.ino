#include <math.h>
#include "Arduino.h"
#include "Enes100.h"

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

const int trigPin = 30;
const int echoPin = 31;

double goalX = 3.2;
double safeY;
float sensorThreshold = 0.15; 
int avoidanceStep = 0; 

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

// --- ACTIVE BRAKING FUNCTION ---
// Counter-acts momentum by firing motors in reverse for a split second
void brake(int lastSpeedL, int lastSpeedR) {
  setMotors(-lastSpeedL, -lastSpeedR);
  delay(45); // Short burst of reverse thrust
  stopAll();
}

double Ultra() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 20000);
  float d = (duration * 0.000343) / 2.0;
  return (d <= 0) ? 5.0 : d;
}

void faceAngle(double targetTheta) {
  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }

    double currentTheta = Enes100.getTheta();
    double error = targetTheta - currentTheta;
    while (error > PI) error -= 2 * PI;
    while (error < -PI) error += 2 * PI;

    if (abs(error) < 0.12) break; // Close enough for high speed

    // HIGH SPEED TURN
    int speed = (abs(error) > 0.4) ? 180 : 130; 
    
    int sL = (error > 0) ? -speed : speed;
    int sR = (error > 0) ? speed : -speed;
    
    setMotors(sL, sR);
    
    // Check frequently, then brake immediately when error is small
    if (abs(error) < 0.25) {
      delay(20); 
      brake(sL, sR); // Kill the spin
    }
  }
  stopAll();
}

void navigateTo(double tx, double ty) {
  while (!Enes100.isVisible());
  double dx = tx - Enes100.getX();
  double dy = ty - Enes100.getY();
  faceAngle(atan2(dy, dx));

  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    double currX = Enes100.getX();
    double currY = Enes100.getY();
    double dist = sqrt(pow(tx - currX, 2) + pow(ty - currY, 2));

    if (dist < 0.15 || Ultra() < sensorThreshold) break;
    setMotors(200, 200); // High speed travel
  }
  stopAll();
}

bool driveForwardWithDetection() {
  faceAngle(0); 
  unsigned long moveStartTime = millis();
  while (millis() - moveStartTime < 3500) { 
    if (!Enes100.isVisible()) { stopAll(); continue; }
    if (Enes100.getX() >= goalX) return true;
    if (Ultra() < sensorThreshold) { stopAll(); return false; }
    setMotors(220, 220); // Aggressive forward
  }
  return true;
}

void avoidObstacle() {
  avoidanceStep++; 
  setMotors(-180, -180); // Rapid backup
  delay(400);
  stopAll();

  double targetY;
  // Systematic lane checking
  if (avoidanceStep == 1) targetY = 1.6; 
  else if (avoidanceStep == 2) targetY = 0.4;
  else targetY = 1.0;

  if (targetY > 1.8) targetY = 1.8;
  if (targetY < 0.2) targetY = 0.2;

  navigateTo(Enes100.getX(), targetY);
}

void setup() {
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(ENC, OUTPUT); pinMode(END, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT);

  Enes100.begin("PRIUS", HYDROGEN, 238, 1210, 10, 11);
  
  while (!Enes100.isVisible()) { delay(50); }
  safeY = Enes100.getY();
  navigateTo(0.6, safeY);
}

void loop() {
  if (Enes100.isVisible() && Enes100.getX() >= goalX) {
    setMotors(-200, -200); delay(100); stopAll(); // Final Brake
    while (true);
  }

  if (!driveForwardWithDetection()) {
    avoidObstacle();
  } else {
    avoidanceStep = 0; 
  }
}