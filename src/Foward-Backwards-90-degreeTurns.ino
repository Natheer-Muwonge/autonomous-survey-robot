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

void setup() {
  Serial.begin(9600);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(ENC, OUTPUT); pinMode(END, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);
}

void forward(int speed) {
  analogWrite(ENA, speed); digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(ENB, speed); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENC, speed); digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW);
  analogWrite(END, speed); digitalWrite(IN7, LOW);  digitalWrite(IN8, HIGH);
}

void backward(int speed) {
  analogWrite(ENA, speed); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(ENB, speed); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(ENC, speed); digitalWrite(IN5, LOW);  digitalWrite(IN6, HIGH);
  analogWrite(END, speed); digitalWrite(IN7, HIGH); digitalWrite(IN8, LOW);
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
  Serial.println("Forward");
  forward(255);
  delay(8000);
  stopAll();
  delay(2000);

  Serial.println("Turn 1 - Left");
  turnLeft(200);
  delay(9000);
  stopAll();
  delay(1000);

  Serial.println("Turn 2 - Right");
  turnRight(200);
  delay(9000);
  stopAll();
  delay(1000);

  Serial.println("Turn 3 - Left");
  turnLeft(200);
  delay(9000);
  stopAll();
  delay(1000);

  Serial.println("Backward");
  backward(255);
  delay(8000);
  stopAll();

  Serial.println("All sub-tasks complete");
  while(true);
}