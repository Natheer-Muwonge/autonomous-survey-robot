#include "Arduino.h"
#include <math.h>
#include "Enes100.h"
#include <Servo.h>

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

// ── Color Sensor ──
#define S0 36
#define S1 37
#define S2 38
#define S3 39
#define sensorOut 40

// ── Servos ──
#define ARM_SERVO_PIN 6
#define MICRO_SERVO1  7
#define MICRO_SERVO2  8

Servo armServo;
Servo microServo1;
Servo microServo2;

double targetY = 0.00;
double PI_VAL = 3.14159265358979;

// ── Phases ──
#define PHASE_NAV1    0   // Navigate to mission site
#define PHASE_MISSION 1   // Drop marbles
#define PHASE_NAV2    2   // Navigate past obstacles
#define PHASE_NAV3    3   // Navigate to destination zone
#define PHASE_DONE    4

int phase = PHASE_NAV1;

// ── Nav1 states ──
#define STATE_ALIGN_Y 0
#define STATE_DRIVE_X 1
#define STATE_AVOID   2

int navState = STATE_ALIGN_Y;
double avoidDirection = PI_VAL / 2;
double avoidStartX = 0;
int avoidanceStep = 0;

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

// ── Ultrasonic ────────────────────────────────
float getDistance() {
  digitalWrite(TRIG1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG1, LOW);
  long duration = pulseIn(ECHO1, HIGH, 30000);
  float d = (duration * 0.034 / 2) / 100.0;
  return (d <= 0) ? 5.0 : d;
}

// ── Angle controller ──────────────────────────
void setAngle(double goalAngle, double curTheta) {
  double error = goalAngle - curTheta;
  while (error > PI_VAL)  error -= 2 * PI_VAL;
  while (error < -PI_VAL) error += 2 * PI_VAL;
  if (abs(error) > 0.15) {
    if (error > 0) turnLeft(150);
    else           turnRight(150);
  } else {
    forward(220);
  }
}

// ── Navigate to point (blocking) ─────────────
void navigateTo(double tx, double ty) {
  while (true) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    double curX = Enes100.getX();
    double curY = Enes100.getY();
    double curTheta = Enes100.getTheta();
    double dx = tx - curX;
    double dy = ty - curY;
    double dist = sqrt(dx*dx + dy*dy);
    if (dist < 0.15) break;
    double goalAngle = (abs(dy) > 0.15) ? ((dy > 0) ? PI_VAL/2 : -PI_VAL/2) : 0;
    setAngle(goalAngle, curTheta);
  }
  stopAll();
}

// ── Color sensor ──────────────────────────────
String readColor() {
  int redFrequency = 0;
  int greenFrequency = 0;
  int blueFrequency = 0;

  digitalWrite(S2, LOW);  digitalWrite(S3, LOW);
  redFrequency = pulseIn(sensorOut, LOW);
  delay(100);
  digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(sensorOut, LOW);
  delay(100);
  digitalWrite(S2, LOW);  digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(sensorOut, LOW);
  delay(100);

  Enes100.print("R:"); Enes100.print(redFrequency);
  Enes100.print(" G:"); Enes100.print(greenFrequency);
  Enes100.print(" B:"); Enes100.println(blueFrequency);

  if (redFrequency < 85 && redFrequency > 25 && greenFrequency < 130 && greenFrequency > 65 && blueFrequency > 75 && blueFrequency < 135) return "YELLOW";
  else if (greenFrequency < 130 && greenFrequency > 85 && redFrequency > 115 && redFrequency < 165 && blueFrequency > 105 && blueFrequency < 145) return "GREEN";
  else if (blueFrequency < 110 && blueFrequency > 70 && redFrequency > 110 && redFrequency < 155 && greenFrequency > 105 && greenFrequency < 150) return "BLUE";
  else if (redFrequency < 100 && redFrequency > 25 && greenFrequency > 110 && greenFrequency < 160 && blueFrequency > 105 && blueFrequency < 155) return "RED";
  else if (redFrequency < 150 && redFrequency > 80 && greenFrequency < 180 && greenFrequency > 80 && blueFrequency < 160 && blueFrequency > 80) return "WHITE";
  else return "UNKNOWN";
}

void sendColor(String color) {
  Enes100.print("Color: "); Enes100.println(color);
  if      (color == "RED")    Enes100.mission(LED_COLOR, RED);
  else if (color == "GREEN")  Enes100.mission(LED_COLOR, GREEN);
  else if (color == "BLUE")   Enes100.mission(LED_COLOR, BLUE);
  else if (color == "YELLOW") Enes100.mission(LED_COLOR, YELLOW);
  else if (color == "WHITE")  Enes100.mission(LED_COLOR, WHITE);
  else Enes100.println("Color unknown");
}

// ── Mission ───────────────────────────────────
void runMission() {
  Enes100.println("Starting mission...");

  armServo.attach(ARM_SERVO_PIN);
  microServo1.attach(MICRO_SERVO1);
  microServo2.attach(MICRO_SERVO2);
  delay(200);
  microServo1.write(0);
  microServo2.write(0);
  armServo.write(0);
  delay(500);

  // Read and transmit color
  String color = readColor();
  sendColor(color);
  delay(500);

  // Extend arm forward to 160
  Enes100.println("Extending arm...");
  for (int pos = 0; pos <= 160; pos++) {
    armServo.write(pos);
    delay(15);
  }
  delay(1000);

  // Drop hydrogen
  Enes100.println("Dropping hydrogen...");
  microServo1.write(90);
  delay(1500);
  microServo1.write(0);
  delay(500);

  // Drop oxygen
  Enes100.println("Dropping oxygen...");
  microServo2.write(90);
  delay(1500);
  microServo2.write(0);
  delay(500);

  // Retract arm
  Enes100.println("Retracting arm...");
  for (int pos = 160; pos >= 0; pos--) {
    armServo.write(pos);
    delay(15);
  }

  // Detach servos so they don't interfere with Nav2/Nav3 motors
  armServo.detach();
  microServo1.detach();
  microServo2.detach();

  Enes100.println("Mission complete");
}

// ── Nav2 obstacle avoidance ───────────────────
bool driveForwardWithDetection() {
  setAngle(0, Enes100.getTheta());
  unsigned long t = millis();
  while (millis() - t < 3000) {
    if (!Enes100.isVisible()) { stopAll(); continue; }
    if (Enes100.getX() >= 3.20) return true;
    if (getDistance() <= 0.20) { stopAll(); return false; }
    setAngle(0, Enes100.getTheta());
  }
  return true;
}

void avoidObstacleNav2() {
  avoidanceStep++;
  // Back up
  setMotor(ENA, IN1, IN2, 180, true);
  setMotor(ENB, IN3, IN4, 180, false);
  setMotor(ENC, IN5, IN6, 180, false);
  setMotor(END, IN7, IN8, 180, true);
  delay(400);
  stopAll();

  // Pick new Y lane
  double ty;
  if (avoidanceStep == 1)      ty = 1.6;
  else if (avoidanceStep == 2) ty = 0.4;
  else                         ty = 1.0;
  ty = constrain(ty, 0.2, 1.8);
  navigateTo(Enes100.getX(), ty);
}

void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(ENC, OUTPUT); pinMode(END, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);
  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);

  pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  digitalWrite(S0, HIGH); digitalWrite(S1, LOW);

  Enes100.begin("PRIUS", HYDROGEN, 238, 1120, 10, 11);
  Enes100.println("Connected — waiting for marker...");

  while (!Enes100.isVisible()) {
    Enes100.println("Marker not visible");
    delay(100);
  }

  double startY = Enes100.getY();
  Enes100.print("Start Y:"); Enes100.println(startY);

  if (startY > 1.0) {
    targetY = 0.50;
    Enes100.println("Target: BOTTOM");
  } else {
    targetY = 1.50;
    Enes100.println("Target: TOP");
  }

  avoidDirection = (targetY < 1.0) ? PI_VAL/2 : -PI_VAL/2;
  Enes100.println("Ready — starting navigation");
}

void loop() {
  if (!Enes100.isVisible()) {
    forward(200); // keep moving if marker briefly lost
    return;
  }

  double curX = Enes100.getX();
  double curY = Enes100.getY();
  double curTheta = Enes100.getTheta();
  float frontDist = getDistance();

  Enes100.print("P:"); Enes100.print(phase);
  Enes100.print(" X:"); Enes100.print(curX);
  Enes100.print(" Y:"); Enes100.print(curY);
  Enes100.print(" D:"); Enes100.println(frontDist);

  switch (phase) {

    // ── PHASE 0: Navigate to mission site ──────
    case PHASE_NAV1:
      // Stop at mission site
      if (frontDist > 0 && frontDist <= 0.15) {
        stopAll();
        Enes100.println("NAV1 COMPLETE");
        phase = PHASE_MISSION;
        return;
      }
      if (curX >= 3.80) {
        stopAll();
        phase = PHASE_MISSION;
        return;
      }

      switch (navState) {
        case STATE_ALIGN_Y:
          if (abs(curY - targetY) < 0.15) {
            Enes100.println("Y aligned");
            navState = STATE_DRIVE_X;
          } else {
            double goalAngle = (targetY > curY) ? PI_VAL/2 : -PI_VAL/2;
            setAngle(goalAngle, curTheta);
          }
          break;

        case STATE_DRIVE_X:
          if (frontDist > 0 && frontDist < 0.25) {
            avoidStartX = curX;
            avoidDirection = (curY < 1.0) ? PI_VAL/2 : -PI_VAL/2;
            Enes100.println("Obstacle — avoiding");
            navState = STATE_AVOID;
          } else {
            if (abs(curY - targetY) > 0.25) {
              double goalAngle = (targetY > curY) ? PI_VAL/4 : -PI_VAL/4;
              setAngle(goalAngle, curTheta);
            } else {
              setAngle(0, curTheta);
            }
          }
          break;

        case STATE_AVOID:
          if (curX > avoidStartX + 0.40) {
            Enes100.println("Obstacle cleared");
            navState = STATE_ALIGN_Y;
          } else {
            double goalAngle = (avoidDirection > 0) ? PI_VAL/3 : -PI_VAL/3;
            setAngle(goalAngle, curTheta);
          }
          break;
      }
      break;

    // ── PHASE 1: Mission ────────────────────────
    case PHASE_MISSION:
      runMission();
      avoidanceStep = 0;
      Enes100.println("Starting NAV2...");
      phase = PHASE_NAV2;
      break;

    // ── PHASE 2: Past obstacles ─────────────────
    case PHASE_NAV2:
      if (curX >= 3.20) {
        stopAll();
        Enes100.println("NAV2 COMPLETE");
        phase = PHASE_NAV3;
        return;
      }
      if (!driveForwardWithDetection()) {
        avoidObstacleNav2();
      } else {
        avoidanceStep = 0;
      }
      break;

    // ── PHASE 3: Destination zone ───────────────
    case PHASE_NAV3:
      navigateTo(3.8, 1.0);
      Enes100.println("NAV3 COMPLETE");
      phase = PHASE_DONE;
      break;

    // ── PHASE 4: Done ───────────────────────────
    case PHASE_DONE:
      stopAll();
      Enes100.println("ALL COMPLETE");
      while (true);
      break;
  }
}