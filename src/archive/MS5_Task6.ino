#include "Arduino.h"
#include "Enes100.h"
#include "Tank.h"


int state = 0;          // Tracks our progress (0: Turn Y, 1: Turn X, 2: Drive, 3: Stop)
bool goingRight = true; // Tracks which way we need to drive

void setup() {
  Tank.begin();
  Enes100.begin("HydrogenTeam", FIRE, 3, 1120, 8, 9);
  delay(1000); 
}

void turnToAngle(float targetAngle) {
  float tolerance = 0.1;   
  bool isAligned = false;  

  while (!isAligned) {
    float currentAngle = Enes100.getTheta();
    float error = targetAngle - currentAngle;

    while (error > PI) error -= 2 * PI;
    while (error < -PI) error += 2 * PI;

    if (abs(error) <= tolerance) {
      Tank.turnOffMotors();  
      isAligned = true;      
      delay(300);            
    } 
    else if (error > 0) {
      Tank.setLeftMotorPWM(-100);
      Tank.setRightMotorPWM(100);
    } else {
      Tank.setLeftMotorPWM(100);
      Tank.setRightMotorPWM(-100);
    }
  }
}

void loop() {
  float currentX = Enes100.getX();
  float currentY = Enes100.getY();

  // STEP 0: Face the Mission Site (Y-Axis)
  if (state == 0) {
    if (currentY > 1.0) {
      turnToAngle(-1.57); // Face Down
    } else {
      turnToAngle(1.57);  // Face Up
    }
    state = 1; 
  } 
  
  // STEP 1: Face the Log/Limbo end (X-Axis)
  else if (state == 1) {
    if (currentX < 2.0) {
      turnToAngle(0.0);   // Face Right
      goingRight = true;
    } else {
      turnToAngle(PI);    // Face Left
      goingRight = false;
    }
    state = 2; 
  }
  
  // STEP 2: Drive straight to the end
  else if (state == 2) {
    Tank.setLeftMotorPWM(255);
    Tank.setRightMotorPWM(255);

    // Stop when we reach the other side
    if (goingRight && currentX > 3.5) {
      state = 3;
    } else if (!goingRight && currentX < 0.5) {
      state = 3;
    }
  }
  
  // STEP 3: Stop forever
  else if (state == 3) {
    Tank.turnOffMotors();
  }
}