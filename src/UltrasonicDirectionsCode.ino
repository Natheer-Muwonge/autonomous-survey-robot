#define ENA 2
#define ENB 3
#define IN1 22
#define IN2 24
#define IN3 26
#define IN4 28

#define ENC 4
#define END 5
#define IN5 23
#define IN6 25
#define IN7 27
#define IN8 29

#define TRIG1 30  // Front
#define ECHO1 31
#define TRIG2 32  // Left
#define ECHO2 33
#define TRIG3 34  // Right
#define ECHO3 35

#define OBSTACLE_DISTANCE 15  // cm — adjust this threshold as needed

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
  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);

  Serial.println("Starting...");
  delay(3000);
}

long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  long dist = duration * 0.034 / 2;
  if (dist == 0) dist = 999; // if no reading treat as clear
  return dist;
}

void forward(int speed) {
  analogWrite(ENA, speed); digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(ENB, speed); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENC, speed); digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW);
  analogWrite(END, speed); digitalWrite(IN7, LOW);  digitalWrite(IN8, HIGH);
}

void turnLeft(int speed) {
  analogWrite(ENA, speed); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(ENB, speed); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(ENC, speed); digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW);
  analogWrite(END, speed); digitalWrite(IN7, LOW);  digitalWrite(IN8, HIGH);
}

void turnRight(int speed) {
  analogWrite(ENA, speed); digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(ENB, speed); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENC, speed); digitalWrite(IN5, LOW);  digitalWrite(IN6, HIGH);
  analogWrite(END, speed); digitalWrite(IN7, HIGH); digitalWrite(IN8, LOW);
}

void stopAll() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  analogWrite(ENC, 0); analogWrite(END, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(IN5, LOW); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW); digitalWrite(IN8, LOW);
}

void loop() {
  long frontDist = getDistance(TRIG1, ECHO1);
  long leftDist  = getDistance(TRIG2, ECHO2);
  long rightDist = getDistance(TRIG3, ECHO3);

  Serial.print("F: "); Serial.print(frontDist);
  Serial.print(" L: "); Serial.print(leftDist);
  Serial.print(" R: "); Serial.println(rightDist);

  // ── Navigation Objective 1: stop within 150mm ──
  if (frontDist <= 15) {
    stopAll();
    Serial.println("Within 150mm — STOPPED");
    while (true); // stop permanently once at mission site
  }

  // ── Obstacle detection and avoidance ──
  else if (frontDist < OBSTACLE_DISTANCE) {
    stopAll();
    delay(300);

    // Check which side has more room and turn that way
    if (rightDist > leftDist) {
      Serial.println("Obstacle ahead — turning right");
      turnRight(200);
      delay(1500); // adjust for ~45 degree turn
    } else {
      Serial.println("Obstacle ahead — turning left");
      turnLeft(200);
      delay(1500);
    }
    stopAll();
    delay(300);
  }

  // ── No obstacle — drive forward ──
  else {
    forward(200);
  }

  delay(100);
}