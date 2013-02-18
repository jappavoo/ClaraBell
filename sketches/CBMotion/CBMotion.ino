#include <PinChangeInt.h>
#include <PinChangeIntConfig.h>

// HAVE ADDED ENCODER INTERRUPT AND COUNTER SUPPORT
// BUT HAVE NOT TESTED IT!!!!

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
#define LEFT  0
#define RIGHT 1

// MOTOR PIN ASSIGNMENTS
#define MOTOR_L_PWM     3 
#define MOTOR_L_DIR     12
#define MOTOR_L_BRAKE   9
#define MOTOR_L_CURRENT A0
#define MOTOR_L_ENCODER A2

#define MOTOR_R_PWM     11 
#define MOTOR_R_DIR     13
#define MOTOR_R_BRAKE   8
#define MOTOR_R_CURRENT A1
#define MOTOR_R_ENCODER A3

int speedCmd;
unsigned long monTimer;
int const monInterval=500;

// FREE PINS AFTER MOTOR ASSIGNMENTS
// D0 D1 D2 D4 D5 D6 D7 D10
// A4 A5

#define KILL_PIN 4
int KillValue; 

class Motor {
  byte  num;
  
  byte  pwm;
  byte  dir;
  byte  brake;
  float current;
  volatile long  encoder;
  
  int   spd;
  int   pwmPin;
  int   dirPin;
  int   brakePin;
  int   currentPin;
  int   encoderPin;
public:
  Motor(int i, int pp, int dp, int bp, int cp, int ep);
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
  void incEncoder(int pin);
};

void encoderFunc();

void 
Motor::setup(void)
{
  V(if (num==LEFT) Serial.println("Left: Setup"); else Serial.println("Right: Setup"));
  pinMode(dirPin, OUTPUT); //Initiates Motor Channel A pin
  pinMode(brakePin, OUTPUT); //Initiates Brake Channel A pin
  pinMode(encoderPin, INPUT); //set the pin to input
  // encoder circuit has a pull up resistor so no write is required to
  // use internal arduino pull up resistor.
  PCintPort::attachInterrupt(encoderPin, encoderFunc, RISING); // attach 
}

void
Motor::fwd(void)
{
  V(if (num==LEFT) Serial.println("Left: fwd"); else Serial.println("Right: fwd"));
  dir=FWDVAL;
  digitalWrite(dirPin, FWDVAL);
}

void
Motor::bwd(void)
{
  V(if (num==LEFT) Serial.println("Left: bwd"); else Serial.println("Right: bwd"));
  dir=BWDVAL;
  digitalWrite(dirPin, BWDVAL);
}

void 
Motor::stop(void)
{
  V(if (num==LEFT) Serial.println("Left: stop"); else Serial.println("Right: stop"));
  brake=HIGH;
  digitalWrite(brakePin, HIGH);
}

void 
Motor::go(void)
{
  V(if (num==LEFT) Serial.println("Left: go"); else Serial.println("Right: go"));
  brake=LOW;
  digitalWrite(brakePin, LOW);
}

void 
Motor::readCurrent(void)
{
  V(if (num==LEFT) Serial.println("Left: readCurrent"); else Serial.println("Right: readCurrent"));
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
  V(if (num==LEFT) Serial.println("Left: speed"); else Serial.println("Right: speed"));
  spd = validateSpeed(s);
  pwm = speedToPwm(spd);
  analogWrite(pwmPin, pwm);
}

void
Motor::reset(void) 
{
   V(if (num==LEFT) Serial.println("Left: reset"); else Serial.println("Right: reset"));
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

Motor::Motor(int i, int pp, int dp, int bp, int cp, int ep)
{
//    if (num==LEFT) Serial.println("Left: Constructor"); else Serial.println("Right: Constructor");
    num=i;
    pwmPin = pp; dirPin=dp; brakePin=bp; currentPin=cp; encoderPin=ep;
    spd = 0; pwm = 0; dir = 0; encoder=0;
}

void 
Motor::info(void)
{
  if (num==LEFT) Serial.print("Left:  "); else Serial.print("Right: ");
  Serial.print("Info: pwmPin="); Serial.print(pwmPin); 
  Serial.print(", dirPin="); Serial.print(dirPin); 
  Serial.print(", brakePin="); Serial.print(brakePin); 
  Serial.print(", currentPin="); Serial.print(currentPin); 
  Serial.print(", encoderPin="); Serial.print(encoderPin);
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
  Serial.print("KillValue=");
  if (KillValue==LOW) Serial.print("LOW  :"); else Serial.print("HIGH :");
  if (num==LEFT) Serial.print("Left:  "); else Serial.print("Right: ");
  Serial.print("State: pwm="); Serial.print(pwm); 
  Serial.print(" speed="); Serial.print(spd);
  Serial.print(" dir="); Serial.print(dir); 
  Serial.print(" brake="); Serial.print(brake); 
  Serial.print(" encoder="); Serial.print(encoder);
  Serial.print(" current="); Serial.println(current);  
}

void
Motor::incEncoder(int pin)
{
  if (pin==encoderPin) encoder++;
}

Motor L(LEFT , MOTOR_L_PWM, MOTOR_L_DIR, MOTOR_L_BRAKE, MOTOR_L_CURRENT, MOTOR_L_ENCODER);
Motor R(RIGHT, MOTOR_R_PWM, MOTOR_R_DIR, MOTOR_R_BRAKE, MOTOR_R_CURRENT, MOTOR_R_ENCODER);

void encoderFunc()
{
  L.incEncoder(PCintPort::arduinoPin);
  R.incEncoder(PCintPort::arduinoPin); 
}

void test(void)
{
  L.speed(2); L.go(); delay(100); L.stop();
  delay(100);
  L.bwd(); L.go(); delay(100); L.stop();
  L.reset();
  
  R.speed(2); R.go(); delay(100); R.stop();
  delay(100);
  R.bwd(); R.go(); delay(100); R.stop();
  R.reset();
}
                
void setup()
{
  Serial.begin(115200);
 
  pinMode(KILL_PIN, INPUT); 
  KillValue = digitalRead(KILL_PIN);
  
  L.setup(); L.reset();
  R.setup(); R.reset(); 
#if 0 
  test();
#endif
}


void loop()
{
  char c,cmd;
 
   KillValue = digitalRead(KILL_PIN);  
  
  if (KillValue==HIGH) { L.stop(); R.stop(); }
  
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
       if (cmd=='\n' || cmd=='\r') return;
       if (cmd=='l' || cmd=='r') {
         Serial.readBytes(&c,1);
         if (cmd=='l') L.doCmd(c);
         else R.doCmd(c);
       } else if (cmd=='m') {
         if (monTimer==0) monTimer = millis() + monInterval;
         else monTimer=0; 
       } else {
          L.doCmd(cmd);
          R.doCmd(cmd);
       }
  }
}  


