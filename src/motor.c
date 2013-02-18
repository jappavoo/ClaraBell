#include "pins.h"

enum MotorDir { MOTOR_DIR_FORWARD=0, MOTOR_DIR_BACKWARD=1 };
enum MotorBrake { MOTOR_BRAKE_ON=HIGH, MOTOR_BRAKE_OFF=LOW };

struct Motor {
  int pwmVal;
  enum MotorDir dir;
  enum MotorBrake brake;
  int current;
} motorA, motorB;

void motorAPWM(int val) 
{
  motorA.pwmVal = val;
  analogWrite(MOTOR_A_PWM, motorA.pwmVal);
}

void motorBPWM(int val) 
{
  motorB.pwmVal = val;
  analogWrite(MOTOR_B_PWM, motorB.pwmVal);
}

void motorABrake(enum MotorBrake s)
{
  motorA.brake = s;
  digitalWrite(motorA.brake);
}

void motorBBrake(enum MotorBrake s)
{
  motorB.brake = s;
  digitalWrite(motorA.brake);
}

void motorADir(enum MotorDir d)
{
  motorA.dir = d;
  digitalWrite(MOTOR_A_DIR, d);
}

void motorBDir(enum MotorDir d)
{
  motorB.dir = d;
  digitalWrite(MOTOR_B_DIR, d);
}

int motorACurrent(void)
{
  motorA.current = analogRead(MOTOR_A_CURRENT);
  // map input voltage to 0-2000mA range
  motorA.current = map(motorA.current, 0, 676, 0, 2000);
  return motorA.current;
}

int motorBCurrent(void)
{
  motorB.current = analogRead(MOTOR_B_CURRENT);
  motorB.current = map(motorB.current, 0, 676, 0, 2000);
  return motorB.current;
}

void setup()
{
  bzero(&motorA, sizeof(motorA));
  bzero(&motorB, sizeof(motorB));


  pinMode(MOTOR_A_PWM, OUTPUT);
  pinMode(MOTOR_A_DIR, OUTPUT);
  pinMode(MOTOR_A_BRAKE, OUTPUT);

  pinMode(MOTOR_B_PWM, OUTPUT);
  pinMode(MOTOR_B_DIR, OUTPUT);
  pinMode(MOTOR_B_BRAKE, OUTPUT);

  motorABrake(MOTOR_BRAKE_ON);
  motorAPWM(0); 
  motorADir(MOTOR_DIR_FORWARD);
  (void) motorACurrent();


  motorBBrake(MOTOR_BRAKE_ON);
  motorBPWM(0); 
  motorBDir(MOTOR_DIR_FORWARD);
  (void) motorBCurrent();
}


// D0 D1 D2 D4 D5 D6 D7 D10
// A2 A3 A4 A5

void loop() 
{
  int pwmVal = motorA.pwmVal + 8;
  if (pwmVal > 255) pwmVal = 0;
 
  Serial.println("A Current: " + String(motorACurrent()));
  Serial.println("B Current: " + String(motorBCurrent()));

  //Channel A 
  //Direction: Clockwise, Brake: OFF
  motorADir(MOTOR_DIR_FORWARD);
  motorAPWM(pwmVal);
  motorABrake(MOTOR_BRAKE_OFF);

  //Channel B
  //Direction: Clockwise, Brake: OFF
  motorBDir(MOTOR_DIR_FORWARD);
  motorBPWM(pwmVal);
  motorBBrake(MOTOR_BRAKE_OFF);

  delay(1000);

  motorABrake(MOTOR_BRAKE_ON);
  motorBBrake(MOTOR_BRAKE_OFF);

  delay(10);

  //Channel A 
  //Direction: Counter-Clockwise, Brake: OFF
  motorADir(MOTOR_DIR_BACKWARD);
  motorAPWM(pwmVal);
  motorABrake(MOTOR_BRAKE_OFF);


  //Channel B
  //Direction: Counter-Clockwise, Brake: OFF
  motorBDir(MOTOR_DIR_BACKWARD);
  motorBPWM(pwmVal);
  motorBBrake(MOTOR_BRAKE_OFF);

  delay(1000);

}

