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

int motorFL_1 = 22; int motorFL_2 = 24; int motorFL_speed = 2;
int motorBL_1 = 26; int motorBL_2 = 28; int motorBL_speed = 3;
int motorFR_1 = 23; int motorFR_2 = 25; int motorFR_speed = 4;
int motorBR_1 = 27; int motorBR_2 = 29; int motorBR_speed = 5;

#define trigPin 30
#define echoPin 31
#define SERVO_PIN 6

Servo Hydrogen;
Servo Oxygen;
Servo Hydro;

// ── SETTINGS ──
const float sensorThresholdNav2 = 0.11;
const float sensorThresholdNav1 = 0.021; // Precision tolerance from second code

const int ARM_UP = 175;
const int ARM_READY = 150;
const int ARM_DOWN = 76;
const int voltagePin = A1;

double targetY = 1.0;
int moveSpeed = 220;
int avoidanceStep = 0;

// ══════════════════════════════════════════════════════════
// MOTOR CONTROL
// ══════════════════════════════════════════════════════════

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

void stopAll() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  analogWrite(ENC, 0);
  analogWrite(END, 0);
}

void brake(int lastSpeedL, int lastSpeedR) {
  setMotors(-lastSpeedL, -lastSpeedR);
  delay(60);
  stopAll();
}

// ══════════════════════════════════════════════════════════
// STRAFE HELPERS
// ══════════════════════════════════════════════════════════

void strafeDrive(int p1, int p2, int spdPin, int speed, bool forward) {
  analogWrite(spdPin, speed);
  digitalWrite(p1, forward ? HIGH : LOW);
  digitalWrite(p2, forward ? LOW : HIGH);
}

void strafeLeft(int speed) {
  strafeDrive(motorFL_1, motorFL_2, motorFL_speed, speed, true);
  strafeDrive(motorBR_1, motorBR_2, motorBR_speed, speed, true);
  strafeDrive(motorBL_1, motorBL_2, motorBL_speed, speed, true);
  strafeDrive(motorFR_1, motorFR_2, motorFR_speed, speed, true);
}

void strafeRight(int speed) {
  strafeDrive(motorFL_1, motorFL_2, motorFL_speed, speed, false);
  strafeDrive(motorBR_1, motorBR_2, motorBR_speed, speed, false);
  strafeDrive(motorBL_1, motorBL_2, motorBL_speed, speed, false);
  strafeDrive(motorFR_1, motorFR_2, motorFR_speed, speed, false);
}

// ══════════════════════════════════════════════════════════
// SENSORS & MISSION DATA
// ══════════════════════════════════════════════════════════

double Ultra() {
  double readings[3];
  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 30000);
    readings[i] = (duration * 0.000343) / 2.0;
    if (readings[i] <= 0.01) readings[i] = 4.0;
    delay(10);
  }
  double minDist = readings[0];
  if (readings[1] < minDist) minDist = readings[1];
  if (readings[2] < minDist) minDist = readings[2];
  return minDist;
}

void Voltage_Read() {
  int sensorValue = analogRead(voltagePin);
  float exactVoltage = sensorValue * (5.0 / 1023.0);

  if ((exactVoltage > 0.9) && (exactVoltage < 1.1)) {
    Enes100.mission(exactVoltage, VOLTAGE_1);
  }
  else if ((exactVoltage > 1.9) && (exactVoltage < 2.1)) {
    Enes100.mission(exactVoltage, VOLTAGE_2);
  }
  else if ((exactVoltage > 2.9) && (exactVoltage < 3.1)) {
    Enes100.mission(exactVoltage, VOLTAGE_3);
  }
  else if ((exactVoltage > 3.9) && (exactVoltage < 4.1)) {
    Enes100.mission(exactVoltage, VOLTAGE_4);
  }
  else if (exactVoltage > 4.9) {
    Enes100.mission(exactVoltage, VOLTAGE_5);
  }
  delay(500);
}

// ══════════════════════════════════════════════════════════
// ANGLE HELPERS
// ══════════════════════════════════════════════════════════

// Tight Version for Mission Start
void faceAngle(double targetTheta) {
  int stableCount = 0;
  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    double currentTheta = Enes100.getTheta();
    double error = targetTheta - currentTheta;
    while (error > PI) error -= 2 * PI;
    while (error < -PI) error += 2 * PI;

    if (abs(error) < 0.02) {
      stableCount++;
      if (stableCount >= 3) break;
      stopAll(); delay(60); continue;
    }
    stableCount = 0;
    int speed = (abs(error) > 0.40) ? 140 : (abs(error) > 0.12 ? 80 : 68);
    int sL = (error > 0) ? -speed : speed;
    int sR = (error > 0) ? speed : -speed;
    setMotors(sL, sR);
    if (abs(error) < 0.15) { delay(15); stopAll(); delay(50); }
  }
  stopAll();
}

// Looser Version for Navigation & Centering
void faceAngleLoose(double targetTheta) {
  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    double currentTheta = Enes100.getTheta();
    double error = targetTheta - currentTheta;
    while (error > PI) error -= 2 * PI;
    while (error < -PI) error += 2 * PI;

    if (abs(error) < 0.05) break; // Wider tolerance for speed

    int speed = (abs(error) > 0.30) ? 110 : (abs(error) > 0.12 ? 80 : 63);
    int sL = (error > 0) ? -speed : speed;
    int sR = (error > 0) ? speed : -speed;
    setMotors(sL, sR);
    if (abs(error) < 0.12) { delay(12); stopAll(); delay(45); }
  }
  stopAll();
}

void correctAngleIfNeeded(double targetTheta) {
  if (!Enes100.isVisible()) return;
  double error = targetTheta - Enes100.getTheta();
  while (error > PI) error -= 2 * PI;
  while (error < -PI) error += 2 * PI;
  if (abs(error) > 0.10) faceAngleLoose(targetTheta);
}

// ══════════════════════════════════════════════════════════
// NAV 1 — ALIGN Y
// ══════════════════════════════════════════════════════════

void nav1_alignY() {
  bool movingUp = (targetY > Enes100.getY());
  double angleToLane = movingUp ? PI / 2 : -PI / 2;
  double stagingY = movingUp ? 0.65 : 1.35;
  double targetX = 0.2585;

  faceAngle(angleToLane);

  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    correctAngleIfNeeded(angleToLane);
    double currentY = Enes100.getY();
    bool reachedStaging = movingUp ? (currentY >= stagingY) : (currentY <= stagingY);
    if (reachedStaging || Ultra() < sensorThresholdNav1) {
      brake(moveSpeed, moveSpeed);
      delay(200);
      break;
    }
    setMotors(moveSpeed, moveSpeed);
  }

  faceAngle(angleToLane);
  delay(400);

  Hydrogen.attach(SERVO_PIN);
  Hydrogen.write(150);
  delay(500);
  Hydrogen.detach();

  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    double currentX = Enes100.getX();
    if (abs(currentX - targetX) < 0.012) { stopAll(); break; }
    if (currentX > targetX) { (movingUp) ? strafeLeft(60) : strafeRight(60); } 
    else { (movingUp) ? strafeRight(60) : strafeLeft(60); }
    delay(10);
  }

  faceAngle(angleToLane);

  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    correctAngleIfNeeded(angleToLane);
    double currentY = Enes100.getY();
    bool reachedTarget = movingUp ? (currentY >= targetY - 0.04) : (currentY <= targetY + 0.04);
    if (reachedTarget || Ultra() < sensorThresholdNav1) {
      brake(moveSpeed, moveSpeed);
      break;
    }
    setMotors(100, 100);
  }

  faceAngle(angleToLane);
  delay(300);
  setMotors(100,100);
  delay(110);
  stopAll();
  Hydrogen.attach(SERVO_PIN);
  Hydrogen.write(180);
  delay(500);
  Hydrogen.detach();
  delay(5000);
 
}

void mission_one() {
  Hydro.attach(7);
  Oxygen.attach(8);
  Hydro.write(90);
  Oxygen.write(90);
  delay(3000);
  Hydro.detach();
  Oxygen.detach();
}

// ══════════════════════════════════════════════════════════
// NAV 1B — CENTER + FACE RIGHT
// ══════════════════════════════════════════════════════════

void nav1b_centerAndFace() {
  double angleToLane = (targetY > 1.0) ? PI / 2 : -PI / 2;
  double reverseTargetY = (targetY > 1.0) ? 0.85 : 1.15;

  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    correctAngleIfNeeded(angleToLane);
    bool reached = (targetY > 1.0) ? (Enes100.getY() <= reverseTargetY) : (Enes100.getY() >= reverseTargetY);
    if (reached) { brake(-180, -180); break; }
    setMotors(-180, -180);
  }

  delay(1000);
  faceAngleLoose(0); // Uses loose tolerance as requested
  Hydrogen.attach(SERVO_PIN);
  Hydrogen.write(ARM_READY);
  delay(500);
  Hydrogen.detach();
}

// ══════════════════════════════════════════════════════════
// NAV 2 — DRIVE X
// ══════════════════════════════════════════════════════════

void avoidObstacleStrafe() {
  avoidanceStep++;
  setMotors(-180, -180);
  delay(400);
  stopAll();

  double nextY;
  if (avoidanceStep == 1) nextY = 1.6;
  else if (avoidanceStep == 2) nextY = 0.4;
  else { nextY = 1.0; avoidanceStep = 0; }

  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    if (abs(Enes100.getY() - nextY) < 0.08) break;
    if (Enes100.getY() < nextY) strafeLeft(200);
    else strafeRight(200);
  }
  stopAll();
  faceAngleLoose(0);
}

void nav2_driveX() {
  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    correctAngleIfNeeded(0);
    if (Enes100.getX() >= 2.5) break;
    if (Ultra() < sensorThresholdNav2) {
      delay(40);
      if (Ultra() < sensorThresholdNav2) {
        stopAll();
        avoidObstacleStrafe();
      }
    } else {
      setMotors(240, 240);
    }
  }

  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    correctAngleIfNeeded(0);
    if (abs(Enes100.getY() - 1.5) < 0.08) break;
    if (Enes100.getY() < 1.5) strafeLeft(200);
    else strafeRight(200);
  }
  stopAll();
  faceAngleLoose(0);
}

// ══════════════════════════════════════════════════════════
// NAV 3 — DEPLOY
// ══════════════════════════════════════════════════════════

void nav3_deploy() {
  Hydrogen.attach(SERVO_PIN);
  for (int pos = ARM_READY; pos >= ARM_DOWN; pos--) {
    Hydrogen.write(pos);
    delay(30);
  }
  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    correctAngleIfNeeded(0);
    if (Enes100.getX() >= 3.0) break;
    setMotors(220, 220);
    delay(8000);
  }
  stopAll();
  Hydrogen.detach();
}

// ══════════════════════════════════════════════════════════
// SETUP & LOOP
// ══════════════════════════════════════════════════════════

void setup() {
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT); pinMode(ENC, OUTPUT); pinMode(END, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT); pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT);

  Enes100.begin("PRIUS", HYDROGEN, 238, 1120, 10, 11);
  while (!Enes100.isVisible());
  targetY = (Enes100.getY() > 1.1) ? 0.5 : 1.5;
}

void loop() {
  nav1_alignY();
  mission_one();
  Voltage_Read(); // Voltage sensing back in

  nav1b_centerAndFace();
  delay(1000);

  nav2_driveX();
  delay(1000);

  nav3_deploy();
  while (true);
} 
