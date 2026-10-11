#include "Arduino.h"
#include <math.h>
#include "Enes100.h"

// ── Drive System L298N #1 (Left Motors) ──
#define ENA 2
#define ENB 3
#define IN1 22
#define IN2 24
#define IN3 26
#define IN4 28

// ── Drive System L298N #2 (Right Motors) ──
#define ENC 4
#define END 5
#define IN5 23
#define IN6 25
#define IN7 27
#define IN8 29

// ── Front Ultrasonic ──
#define TRIG1 30
#define ECHO1 31

double targetY = 0.00;
double PI_VAL = 3.14159265358979;

#define STATE_ALIGN_Y    0
#define STATE_DRIVE_X    1
#define STATE_AVOID      2

int state = STATE_ALIGN_Y;
double avoidDirection = PI_VAL / 2;
double avoidStartX = 0;

void setup() {
  Serial.begin(9600);

  // Motor pins
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(ENC, OUTPUT); pinMode(END, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);

  // Ultrasonic pins
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  Enes100.begin("PRIUS", HYDROGEN, 238, 1210, 10, 11);
  Enes100.println("Connected — waiting for marker...");

  while (!Enes100.isVisible()) {
    Enes100.println("Marker not visible");
    delay(100);
  }

  double startY = Enes100.getY();
  double startX = Enes100.getX();
  double startTheta = Enes100.getTheta();

  Enes100.print("Start X: "); Enes100.print(startX);
  Enes100.print(" Y: "); Enes100.print(startY);
  Enes100.print(" Theta: "); Enes100.println(startTheta);

  if (startY > 1.0) {
    targetY = 0.50;
    Enes100.println("Target: BOTTOM");
  } else {
    targetY = 1.50;
    Enes100.println("Target: TOP");
  }
}

// ── Motor helpers ─────────────────────────────
void setMotor(int en, int in1, int in2, int speed, bool fwd) {
  analogWrite(en, speed);
  digitalWrite(in1, fwd ? HIGH : LOW);
  digitalWrite(in2, fwd ? LOW : HIGH);
}

void forward(int speed) {
  setMotor(ENA, IN1, IN2, speed, false);
  setMotor(ENB, IN3, IN4, speed, true);
  setMotor(ENC, IN5, IN6, speed, true);
  setMotor(END, IN7, IN8, speed, false);
}

void turnLeft(int speed) {
  setMotor(ENA, IN1, IN2, speed, true);
  setMotor(ENB, IN3, IN4, speed, false);
  setMotor(ENC, IN5, IN6, speed, true);
  setMotor(END, IN7, IN8, speed, false);
}

void turnRight(int speed) {
  setMotor(ENA, IN1, IN2, speed, false);
  setMotor(ENB, IN3, IN4, speed, true);
  setMotor(ENC, IN5, IN6, speed, false);
  setMotor(END, IN7, IN8, speed, true);
}

void stopAll() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  analogWrite(ENC, 0); analogWrite(END, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(IN5, LOW); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW); digitalWrite(IN8, LOW);
}

// ── Ultrasonic distance ───────────────────────
float getDistance() {
  digitalWrite(TRIG1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG1, LOW);
  long duration = pulseIn(ECHO1, HIGH, 30000);
  return (duration * 0.034 / 2) / 100.0; // convert cm to meters
}

// ── Angle controller ──────────────────────────
void setAngle(double goalAngle, double curTheta) {
  double error = goalAngle - curTheta;
  while (error > PI_VAL)  error -= 2 * PI_VAL;
  while (error < -PI_VAL) error += 2 * PI_VAL;

  if (abs(error) > 0.15) {
    if (error > 0) {
      turnLeft(130);
    } else {
      turnRight(130);
    }
  } else {
    forward(180);
  }
}

void loop() {
  if (!Enes100.isVisible()) {
    stopAll();
    Enes100.println("Marker lost — stopped");
    return;
  }

  float frontDist = getDistance();
  double curX = Enes100.getX();
  double curY = Enes100.getY();
  double curTheta = Enes100.getTheta();

  Enes100.print("State: "); Enes100.print(state);
  Enes100.print(" X: "); Enes100.print(curX);
  Enes100.print(" Y: "); Enes100.print(curY);
  Enes100.print(" Theta: "); Enes100.print(curTheta);
  Enes100.print(" Dist: "); Enes100.println(frontDist);

  // ── Stop within 150mm using ultrasonic ──
  if (frontDist > 0 && frontDist <= 0.15) {
    stopAll();
    Enes100.println("Within 150mm — STOPPED");
    while (true);
  }

  // ── Backup stop using X position ──
  if (curX >= 3.80) {
    stopAll();
    Enes100.println("REACHED MISSION SITE");
    while (true);
  }

  switch (state) {

    case STATE_ALIGN_Y:
      Enes100.print("Aligning Y — target: ");
      Enes100.print(targetY);
      Enes100.print(" current: ");
      Enes100.println(curY);
      if (abs(curY - targetY) < 0.15) {
        Enes100.println("Y aligned — driving forward");
        state = STATE_DRIVE_X;
      } else {
        double goalAngle = (targetY > curY) ? PI_VAL/2 : -PI_VAL/2;
        setAngle(goalAngle, curTheta);
      }
      break;

    case STATE_DRIVE_X:
      Enes100.print("Driving X — dist: ");
      Enes100.println(frontDist);
      if (frontDist > 0 && frontDist < 0.30) {
        avoidStartX = curX;
        avoidDirection = (curY < 1.0) ? PI_VAL/2 : -PI_VAL/2;
        Enes100.println("Obstacle — avoiding");
        state = STATE_AVOID;
      } else {
        if (abs(curY - targetY) > 0.20) {
          double goalAngle = (targetY > curY) ? PI_VAL/4 : -PI_VAL/4;
          setAngle(goalAngle, curTheta);
        } else {
          setAngle(0, curTheta);
        }
      }
      break;

    case STATE_AVOID:
      Enes100.print("Avoiding — avoidStartX: ");
      Enes100.print(avoidStartX);
      Enes100.print(" curX: ");
      Enes100.println(curX);
      if (curX > avoidStartX + 0.35) {
        Enes100.println("Obstacle cleared — realigning");
        state = STATE_ALIGN_Y;
      } else {
        double goalAngle = (avoidDirection > 0) ? PI_VAL/3 : -PI_VAL/3;
        setAngle(goalAngle, curTheta);
      }
      break;
  }
}