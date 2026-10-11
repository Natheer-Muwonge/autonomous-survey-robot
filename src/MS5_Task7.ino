#include "Enes100.h"

int state = 0;              // 0: Drive, 1: Turn to Dodge, 2: Bypass, 3: Turn Back, 4: Resume
float originalAngle = 0.0;  // Will store our heading before we dodged
float dodgeAngle = 0.0;     // Will store the 90-degree offset angle

void setup() {
  Tank.begin();
  
  // Replace with your team details
  Enes100.begin("HydrogenTeam", HYDROGEN, 3, 1120, 8, 9);
  
  delay(1000); 
}

// Our reliable turning function
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
  // STATE 0: Drive straight and scan for obstacles
  if (state == 0) {
    Tank.setLeftMotorPWM(150);
    Tank.setRightMotorPWM(150);

    float distance = Tank.readDistanceSensor(1);
    
    // If an obstacle is detected within 0.4 meters
    if (distance > 0.0 && distance < 0.4) {
      Tank.turnOffMotors();
      
      // 1. Record our current heading so we can return to it later
      originalAngle = Enes100.getTheta();
      
      // 2. Calculate a dodge angle (Add PI/2 radians, which is 90 degrees)
      dodgeAngle = originalAngle + (PI / 2.0);
      
      // 3. Keep the angle normalized between -PI and PI
      if (dodgeAngle > PI) dodgeAngle -= 2 * PI;
      
      state = 1; // Move to the turning state
    }
  }
  
  // STATE 1: Turn 90 degrees to face away from the obstacle
  else if (state == 1) {
    turnToAngle(dodgeAngle);
    state = 2;
  }
  
  // STATE 2: Drive forward to bypass the obstacle
  else if (state == 2) {
    Tank.setLeftMotorPWM(150);
    Tank.setRightMotorPWM(150);
    
    // Drive blindly for 1.5 seconds to clear the width of the block
    delay(1500); 
    
    Tank.turnOffMotors();
    state = 3;
  }
  
  // STATE 3: Turn back to our original heading
  else if (state == 3) {
    turnToAngle(originalAngle);
    state = 4;
  }
  
  // STATE 4: Resume driving straight forever
  else if (state == 4) {
    Tank.setLeftMotorPWM(150);
    Tank.setRightMotorPWM(150);
    
    // (Optional: You could add another distance check here to avoid multiple obstacles)
  }
}