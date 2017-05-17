// unified motor control and sonar sensing on to a single arduino
// removed wheel encoder support until we actually build the hardware
// removed kill pin as it is no longer needed from the sonar board
// might add it back for bumper switches 


// Summary of resource usage
//  PIN:   USAGE
//   D0     FREE
//   D1     FREE
//   D2     FREE
//   D3     LEFT MOTOR PWM    (TIMER 2: PWM)
//   D4     PING
//   D5     PING 
//   D6     PING
//   D7     PING
//   D8     RIGHT MOTOR BRAKE
//   D9     LEFT MOTOR BRAKE
//   D10    CAMERA TILT SERVO     (TIMER 1: via servo library)
//   D11    RIGHT MOTOR PWM       (TIMER 2: PWM)
//   D12    LEFT MOTOR DIRECTION
//   D13    RIGHT MOTOR DIRECTION

//   A0     LEFT MOTOR CURRENT SENSE
//   A1     RIGTH MOTOR CURRENT SENSE
//   A2     FREE
//   A3     FREE
//   A4     FREE
//   A5     FREE

// small useful 6 volt moror min
#define PWMMIN 54
#define FWDVAL LOW
#define BWDVAL HIGH
#define PWMINC (255 - PWMMIN)/9

// resolution according to hardware page
#define VOLTS_PER_AMP 1.65

//#define VERBOSE 1

#ifdef VERBOSE
#define V(stmt) { stmt; }
#else
#define V(stmt)
#endif

#define NUM_MOTORS 2
#define LEFT_MOTOR  0
#define RIGHT_MOTOR 1

// MOTOR PIN ASSIGNMENTS
#define MOTOR_L_PWM     3   // timer_2
#define MOTOR_L_DIR     12
#define MOTOR_L_BRAKE   9
#define MOTOR_L_CURRENT A0


#define MOTOR_R_PWM     11 // timer_2
#define MOTOR_R_DIR     13
#define MOTOR_R_BRAKE   8
#define MOTOR_R_CURRENT A1

int speedCmd;
unsigned long monTimer;
int const monInterval=500;

// FREE PINS AFTER MOTOR ASSIGNMENTS
// D0 D1 D2 D4 D5 D6 D7 D10
// A2 A3 A4 A5

class Motor {
  byte  num;
  
  byte  pwm;
  byte  dir;
  byte  brake;
  float current;
  
  int   spd;
  int   pwmPin;
  int   dirPin;
  int   brakePin;
  int   currentPin;

public:
  Motor(int i, int pp, int dp, int bp, int cp);
  void setup(void);
  void reset(void);
  void status(void);
  void stop(void);
  void go(void);
  void speed(int s);
  void fwd(void);
  void bwd(void);
  void readCurrent(void);
  void doCmd(char c);
  void info();
};

void 
Motor::setup(void)
{
  V(if (num==LEFT_MOTOR) Serial.println("Left: Setup"); else Serial.println("Right: Setup"));
  pinMode(dirPin, OUTPUT); //Initiates Motor Channel A pin
  pinMode(brakePin, OUTPUT); //Initiates Brake Channel A pin
}

void
Motor::fwd(void)
{
  V(if (num==LEFT_MOTOR) Serial.println("Left: fwd"); else Serial.println("Right: fwd"));
  dir=FWDVAL;
  digitalWrite(dirPin, FWDVAL);
}

void
Motor::bwd(void)
{
  V(if (num==LEFT_MOTOR) Serial.println("Left: bwd"); else Serial.println("Right: bwd"));
  dir=BWDVAL;
  digitalWrite(dirPin, BWDVAL);
}

void 
Motor::stop(void)
{
  V(if (num==LEFT_MOTOR) Serial.println("Left: stop"); else Serial.println("Right: stop"));
  brake=HIGH;
  digitalWrite(brakePin, HIGH);
}

void 
Motor::go(void)
{
  V(if (num==LEFT_MOTOR) Serial.println("Left: go"); else Serial.println("Right: go"));
  brake=LOW;
  digitalWrite(brakePin, LOW);
}

void 
Motor::readCurrent(void)
{
  V(if (num==LEFT_MOTOR) Serial.println("Left: readCurrent"); else Serial.println("Right: readCurrent"));
  current=(analogRead(currentPin) * (5.0/1024.0))/VOLTS_PER_AMP;
}

int 
validateSpeed(int s) 
{
  if (s<0) return 0;
  if (s>10) return 10;
  return s;
}

int
speedToPwm(int s) 
{
  if (s==0) return 0; else return (PWMMIN + ((s-1)*PWMINC));
}

void
Motor::speed(int s)
{
  V(if (num==LEFT_MOTOR) Serial.println("Left: speed"); else Serial.println("Right: speed"));
  spd = validateSpeed(s);
  pwm = speedToPwm(spd);
  analogWrite(pwmPin, pwm);
}

void
Motor::reset(void) 
{
   V(if (num==LEFT_MOTOR) Serial.println("Left: reset"); else Serial.println("Right: reset"));
   stop();
   speed(0);
   fwd();
   readCurrent();
}

void
Motor::doCmd(char c)
{
  switch (c) {
   case 'S':
     status(); 
     break;
   case 'I':
     info();
     break;
   case 'R':
     reset();
     break;
   case 'H':
     stop();
     speed(0);
     break;  
   case 'g':
     go();
     break;
   case 'h':
     stop();
     break;
   case 'f':
     fwd();
     break;
   case 'b':
     bwd();
     break;  
   case 's':
     if (speedCmd<0) { 
       char d;  
       Serial.readBytes(&d,1);
       speedCmd=d;
     }
     if (speedCmd=='-') speed(0);
     else if (speedCmd>='0' && speedCmd<='9') speed((speedCmd-'0')+1); 
     break; 
   case '-':
     speed(0);
     break;
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':   
   case '9':
     speed((c-'0')+1);
     break;
  case 'a':
     speed(spd+1);
     break;
  case 'd':
     speed(spd-1);
     break;  
  default:
     Serial.print("unknown command:"); Serial.println(c);
  }
}

Motor::Motor(int i, int pp, int dp, int bp, int cp)
{
//    if (num==LEFT) Serial.println("Left: Constructor"); else Serial.println("Right: Constructor");
    num=i;
    pwmPin = pp; dirPin=dp; brakePin=bp; currentPin=cp;
    spd = 0; pwm = 0; dir = 0;
}

void 
Motor::info(void)
{
  if (num==LEFT_MOTOR) Serial.print("Left:  "); else Serial.print("Right: ");
  Serial.print("Info: pwmPin="); Serial.print(pwmPin); 
  Serial.print(", dirPin="); Serial.print(dirPin); 
  Serial.print(", brakePin="); Serial.print(brakePin); 
  Serial.print(", currentPin="); Serial.print(currentPin); 
  Serial.print(", pwmMin="); Serial.print(PWMMIN);
  Serial.print(", pwmInc="); Serial.print(PWMINC);
  Serial.print(", s(0,1,5,10)=("); 
  Serial.print(speedToPwm(0));Serial.print(",");
  Serial.print(speedToPwm(1));Serial.print(",");
  Serial.print(speedToPwm(5));Serial.print(",");
  Serial.print(speedToPwm(10));
  Serial.println(")");
}

void
Motor::status(void) 
{
  if (num==LEFT_MOTOR) Serial.print("Left:  "); else Serial.print("Right: ");
  Serial.print("State: pwm="); Serial.print(pwm); 
  Serial.print(" speed="); Serial.print(spd);
  Serial.print(" dir="); Serial.print(dir); 
  Serial.print(" brake="); Serial.print(brake); 
  Serial.print(" current="); Serial.println(current);  
}

Motor L(LEFT_MOTOR , MOTOR_L_PWM, MOTOR_L_DIR, MOTOR_L_BRAKE, MOTOR_L_CURRENT);
Motor R(RIGHT_MOTOR, MOTOR_R_PWM, MOTOR_R_DIR, MOTOR_R_BRAKE, MOTOR_R_CURRENT);

void 
testMotors()
{
  L.speed(5); L.go(); delay(500); L.stop();
  delay(500);
  L.bwd(); L.go(); delay(500); L.stop();
  L.reset();
  
  R.speed(5); R.go(); delay(500); R.stop();
  delay(500);
  R.bwd(); R.go(); delay(500); R.stop();
  R.reset();
}
                
#include <NewPing.h>

int pwarning   = 20;
int palert     = 15;

#define SONAR_NUM     4 // Number or sensors
#define MAX_DISTANCE 200 // Maximum distance (in cm) for ping routines.  We high pass filter down to 200
#define PING_INTERVAL 33 // Milliseconds between sensor pings (33ms is about the min to avoid cross-sensor echo).

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
unsigned int old[SONAR_NUM];
unsigned int last[SONAR_NUM];
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.
boolean PingAlarm = true;
int PingProx = 0;
boolean PingUpdate = false;

#define FRONTPIN 5
#define RIGHTPIN 4
#define LEFTPIN  6
#define BACKPIN  7


// FREE PINS AFTER MOTOR and SONAR ASSIGNMENTS
// D0 D1 D2 D10
// A2 A3 A4 A5


NewPing sonar[SONAR_NUM] = { 
  NewPing(RIGHTPIN, RIGHTPIN, MAX_DISTANCE),   // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(FRONTPIN, FRONTPIN, MAX_DISTANCE),
  NewPing(LEFTPIN, LEFTPIN, MAX_DISTANCE),    
  NewPing(BACKPIN, BACKPIN, MAX_DISTANCE)
};

#define RIGHT_PING_IDX 0
#define FRONT_PING_IDX 1
#define LEFT_PING_IDX  2
#define BACK_PING_IDX  3

void 
PingSetup() {
  for (uint8_t i=0; i< SONAR_NUM; i++) { old[i]=MAX_DISTANCE; last[i]=0; }
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) { // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  }
}

void 
oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
 if (PingUpdate) {
    Serial.print(last[FRONT_PING_IDX]); Serial.print(" ");
    Serial.print(last[RIGHT_PING_IDX]); Serial.print(" ");
    Serial.print(last[BACK_PING_IDX]); Serial.print(" ");
    Serial.print(last[LEFT_PING_IDX]); Serial.print(" ");
    Serial.print(PingProx);
    Serial.print("\n");
  }
}

void 
PingLoop() {    
  if (millis() >= pingTimer[currentSensor]) {               // Is it this sensor's time to ping?
      cm[currentSensor] = sonar[currentSensor].ping_cm();                // do measurement -- this blocks
      //Serial.print(currentSensor); Serial.print(": "); Serial.println(cm[currentSensor]);
      if (cm[currentSensor] != 0) {
        old[currentSensor]=cm[currentSensor];
        if (old[currentSensor]<pwarning) { PingProx |= 1<<(SONAR_NUM + currentSensor); }
        if (old[currentSensor]<=palert)  {  PingAlarm=true; PingProx |= 1<<currentSensor; } 
        if (old[currentSensor]!=last[currentSensor]) { last[currentSensor]=old[currentSensor]; PingUpdate=true;}
        if ((PingProx & 0x0F)==0) PingAlarm=false;
      }
      pingTimer[currentSensor] = millis() + PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (currentSensor == SONAR_NUM - 1) {
          oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
          currentSensor = 0; // start over again
          PingProx = 0;
          PingUpdate = false;
      } else {
        currentSensor++;      
      }
  }
}


// FREE PINS AFTER MOTOR and SONAR ASSIGNMENTS and camera servo
// D0 D1 D2 
// A2 A3 A4 A5
#define CAMERA_TILT_SERVO_PIN 10
#define CAMERA_TILT_SERVO_MIN 544
#define CAMERA_TILT_SERVO_MAX 2400

#include <Servo.h>

#define CAMERA_TILT_MIN_VAL 60
#define CAMERA_TILT_MAX_VAL 160
#define CAMERA_TILT_STRAIGHT_VAL 90 


// servo library uses timer 1

Servo cameraTiltServo;  // create servo object to control a servo
int CameraTiltAngle = CAMERA_TILT_STRAIGHT_VAL;

void
cameraTiltServoUp() {
   CameraTiltAngle = CAMERA_TILT_MAX_VAL;
   cameraTiltServo.write(CameraTiltAngle);   
}


void
cameraTiltServoDown() {
   CameraTiltAngle = CAMERA_TILT_MIN_VAL;
   cameraTiltServo.write(CameraTiltAngle);   
}

void
cameraTiltServoStraight() {
   CameraTiltAngle = CAMERA_TILT_STRAIGHT_VAL;
   cameraTiltServo.write(CameraTiltAngle);   
}

void
cameraTiltServoInc() {
  if (CameraTiltAngle < CAMERA_TILT_MAX_VAL) {
    CameraTiltAngle += 1;
    cameraTiltServo.write(CameraTiltAngle);   
  }
}

void
cameraTiltServoDec() {
   if (CameraTiltAngle > CAMERA_TILT_MIN_VAL) { 
      CameraTiltAngle-=1;
      cameraTiltServo.write(CameraTiltAngle);
   }
}

void 
cameraTiltValue(long val)
{
  if (val >= CAMERA_TILT_MIN_VAL && val <= CAMERA_TILT_MAX_VAL) {
    CameraTiltAngle=val;		 
    cameraTiltServo.write(CameraTiltAngle);
  } 
}

void
cameraTiltServoSetup()
{
  cameraTiltServo.attach(CAMERA_TILT_SERVO_PIN, CAMERA_TILT_SERVO_MIN, CAMERA_TILT_SERVO_MAX);  
  //Serial.println("cameraTiltServoTest: START");
  cameraTiltServoStraight();
}


void
cameraTiltServoTest()
{
//  cameraTiltServo.write(0);
//  delay(30);
//  Serial.println("cameraTiltServoTest: START");
  cameraTiltServoUp(); 
  delay(1000);
  cameraTiltServoDown();
  delay(1000);
  cameraTiltServoStraight(); 
  delay(500);
//  Serial.println("cameraTiltServoTest: END");
}

void 
setup()
{
  Serial.begin(115200);
 
  PingSetup();
  cameraTiltServoSetup();
 
  cameraTiltServoTest();
  
  L.setup(); L.reset();
  R.setup(); R.reset(); 
#if 0 
  testMotors();
#endif
}

void 
loop()
{
  char c,cmd;
 
  PingLoop();  
  
  if (PingAlarm == true) { L.stop(); R.stop(); }
  
  if (monTimer && millis() > monTimer) {
     L.readCurrent();
     R.readCurrent();
     L.status(); R.status();
     monTimer = millis() + monInterval;
  } 
  
  speedCmd = -1;
      
  if (Serial.available() >= 1) {
       // read the incoming byte:
       cmd=Serial.read();
     //  Serial.println(cmd);
       switch (cmd) {
        case '\n':
        case '\r':
          return;
          break;
        case 'U':
          cameraTiltServoUp();
          break;
        case 'D':
          cameraTiltServoDown();
          break;
        case '=':
          cameraTiltServoStraight();
          break;
        case '+':
          cameraTiltServoInc();
          break;
        case '-':
          cameraTiltServoDec();
          break;
        case 'C':
          {
	          long val = Serial.parseInt();
	          cameraTiltValue(val);
	        }
	        break;  
        case 'm':
          if (monTimer==0) monTimer = millis() + monInterval;
          else monTimer=0; 
          break;
        case 'l':
        case 'r':
          Serial.readBytes(&c,1);
          if (cmd=='l') L.doCmd(c);
          else R.doCmd(c); 
          break;
        default:
          L.doCmd(cmd);
          R.doCmd(cmd);
       }
  }
} 

