#include "Arduino.h"
#include "Enes100.h"

// ── MOTOR PINS ──
int motorFL_1 = 22; int motorFL_2 = 24; int motorFL_speed = 2; 
int motorBL_1 = 26; int motorBL_2 = 28; int motorBL_speed = 3; 
int motorFR_1 = 23; int motorFR_2 = 25; int motorFR_speed = 4; 
int motorBR_1 = 27; int motorBR_2 = 29; int motorBR_speed = 5; 

// ── TCS3200 Color Sensor Pins ──
#define S0 36
#define S1 37
#define S2 38
#define S3 39
#define sensorOut 40

int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// ── TIMING LOGIC ──
unsigned long scanTimer = 0;      // Controls overall Left/Right direction
unsigned long pauseTimer = 0;     // Controls the "jumping" (on/off) bursts
bool strafeLeftDir = true; 
bool isMoving = true;             // Current state of the "jump"

// ── Movement Settings ──
const int BASE_SPEED = 50;       
const int OFFSET_SPEED = 50;     
const int OSCILLATION_TIME = 5000; // Total time spent moving in one general direction
const int MOVE_BURST_TIME = 600;   // How long it "jumps" (moves)
const int PAUSE_BURST_TIME = 1000; // How long it "stops" (waits)

void setup() {
  pinMode(motorFL_1, OUTPUT); pinMode(motorFL_2, OUTPUT); pinMode(motorFL_speed, OUTPUT);
  pinMode(motorBL_1, OUTPUT); pinMode(motorBL_2, OUTPUT); pinMode(motorBL_speed, OUTPUT);
  pinMode(motorFR_1, OUTPUT); pinMode(motorFR_2, OUTPUT); pinMode(motorFR_speed, OUTPUT);
  pinMode(motorBR_1, OUTPUT); pinMode(motorBR_2, OUTPUT); pinMode(motorBR_speed, OUTPUT);

  pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW); 

  Enes100.begin("PRIUS", HYDROGEN, 238, 1120, 10, 11);
  
  scanTimer = millis();
  pauseTimer = millis();
}

// ── Movement Helpers ──────────────────────────
void drive(int p1, int p2, int spdPin, int speed, bool forward) {
  analogWrite(spdPin, speed);
  digitalWrite(p1, forward ? HIGH : LOW);
  digitalWrite(p2, forward ? LOW : HIGH);
}

void strafeLeft(int fast, int slow) {
  drive(motorFL_1, motorFL_2, motorFL_speed, fast, true); 
  drive(motorBR_1, motorBR_2, motorBR_speed, fast, true);  
  drive(motorBL_1, motorBL_2, motorBL_speed, slow, true); 
  drive(motorFR_1, motorFR_2, motorFR_speed, slow, true); 
}

void strafeRight(int fast, int slow) {
  drive(motorFL_1, motorFL_2, motorFL_speed, fast, false); 
  drive(motorBR_1, motorBR_2, motorBR_speed, fast, false); 
  drive(motorBL_1, motorBL_2, motorBL_speed, slow, false);  
  drive(motorFR_1, motorFR_2, motorFR_speed, slow, false);  
}

void stopAll() {
  analogWrite(motorFL_speed, 0); analogWrite(motorBL_speed, 0);
  analogWrite(motorFR_speed, 0); analogWrite(motorBR_speed, 0);
}

// ── Color Sensing ─────────────────────────────
void updateColor() {
  digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  redFrequency = pulseIn(sensorOut, LOW);
  
  digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(sensorOut, LOW);
  
  digitalWrite(S2, LOW); digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(sensorOut, LOW);

  // Calibration Logic
  if (redFrequency < 270 && greenFrequency < 270 && blueFrequency < 200 && abs(redFrequency - greenFrequency) < 35) {
    Enes100.println("WHITE");
  } 
  else if (redFrequency < 100 && (greenFrequency - redFrequency) > 50) {
    Enes100.println("RED");
  } 
  else if (blueFrequency < 130 && blueFrequency < redFrequency && blueFrequency < greenFrequency) {
    Enes100.println("BLUE");
  } 
  else if (redFrequency < 110 && greenFrequency < 160 && blueFrequency > (redFrequency + 50)) {
    Enes100.println("YELLOW");
  } 
  else if (greenFrequency < 135 && redFrequency > (greenFrequency + 40)) {
    Enes100.println("GREEN");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. DIRECTION LOGIC: Swap Left/Right every few seconds
  if (currentMillis - scanTimer > OSCILLATION_TIME) {
    strafeLeftDir = !strafeLeftDir;
    scanTimer = currentMillis;
  }

  // 2. STUTTER LOGIC: Toggle movement on and off
  if (isMoving && (currentMillis - pauseTimer > MOVE_BURST_TIME)) {
    isMoving = false;
    pauseTimer = currentMillis;
  } 
  else if (!isMoving && (currentMillis - pauseTimer > PAUSE_BURST_TIME)) {
    isMoving = true;
    pauseTimer = currentMillis;
  }

  // 3. EXECUTION
  if (isMoving) {
    if (strafeLeftDir) strafeLeft(BASE_SPEED, OFFSET_SPEED);
    else strafeRight(BASE_SPEED, OFFSET_SPEED);
  } else {
    stopAll();
  }

  updateColor();
}