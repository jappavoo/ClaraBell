#include <TimerOne.h>
#include <NewPing.h>


#define PIN_D0  00
#define PIN_D1  01
#define PIN_D2  02
#define PIN_D3  03
#define PIN_D4  04
#define PIN_D5  05
#define PIN_D6  06
#define PIN_D7  07
#define PIN_D8  08
#define PIN_D9  09
#define PIN_D10 10
#define PIN_D11 12
#define PIN_D12 11
#define PIN_D13 13

#define PIN_A0  00
#define PIN_A1  01
#define PIN_A2  02
#define PIN_A3  03
#define PIN_A4  04
#define PIN_A5  05

// MOTOR PIN ASSIGNMENTS
#define MOTOR_A_PWM     PIN_D3 
#define MOTOR_A_DIR     PIN_D12
#define MOTOR_A_BRAKE   PIN_D9
#define MOTOR_A_CURRENT PIN_A0

#define MOTOR_B_PWM     PIN_D11 
#define MOTOR_B_DIR     PIN_D13
#define MOTOR_B_BRAKE   PIN_D8
#define MOTOR_B_CURRENT PIN_A1

// FREE PINS AFTER MOTOR ASSIGNMENTS
// D0 D1 D2 D4 D5 D6 D7 D10
// A2 A3 A4 A5


// SONAR PIN ASSIGNMENTS
#define PING0 PIN_D4
#define PING1 PIN_D5
#define PING2 PIN_D6
#define PING3 PIN_D7


// SONAR CODE IS MODELED after NewPing15Sensors from newping library
#define SONAR_NUM 4
#define MAX_DISTANCE 400

volatile unsigned int sonar_readings[SONAR_NUM];   // Where the ping distances are stored.
#define SONAR_READY  0
#define SONAR_TRIGGERED 1
volatile byte sonar_state;
#define SONAR_SWEEP_TIME 100

unsigned long sonar_sweep_timer;

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(PING0, PING0, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(PING1, PING1, MAX_DISTANCE),
  NewPing(PING2, PING2, MAX_DISTANCE),
  NewPing(PING3, PING3, MAX_DISTANCE),
};

volatile byte sonar_idx;
volatile byte sonar_sweep_active;
// Cmd Processing Variables
#define CMDLEN 4
byte cmd[CMDLEN];

void setup()
{
  Serial.begin(115200); 
  
  sonar_idx = SONAR_NUM;     // initialized to ready state
  sonar_sweep_active = 0;
  sonar_state = SONAR_READY;   
  sonar_sweep_timer = millis() + 100;
  Timer1.initialize(ECHO_TIMER_FREQ);   
}

#define REAL_SONAR_SENSORS 1
void doSonar()
{ 
  // called on every timer interrupt and uses these periodic call
  // to do a sweeps through the sonar sensors gathering a ping value for each sensor
  // Once the sweep is done (sonar_idx == SONAR_NUM) then it stops the sweep by disabling
  // the interrupt. sonar_sweep_active tracks the activity of the sweep. After the values are consumed a new sweep can be started by 
  // resetting the sonar_idx to 0 and reanbling the interrupt.
  // Since the period drives how often we check for and echo it drives the accuracy in which
  // we can detected distances.  According to the NewPing lib ECHO_TIMER_FREQ (24uS) will
  // yield an accuracy of about 0.4cm.
  int rc;
  
  if (sonar_idx < SONAR_NUM) {
    NewPing *s = &sonar[sonar_idx];
    switch (sonar_state) {
    case SONAR_READY:
      sonar_readings[sonar_idx] = 0; // Make reading zero in case there's no ping echo 
#if REAL_SONAR_SENSORS       
      while  (!s->ping_trigger()); 
      sonar_state = SONAR_TRIGGERED;
#else
      sonar_state = SONAR_TRIGGERED;
#endif
      break;
    case SONAR_TRIGGERED:
#if REAL_SONAR_SENSORS    
      rc = s->check();
      if (rc != 0) {
	if (rc==1) sonar_readings[sonar_idx] = s->ping_result;
	sonar_idx++;
        sonar_state = SONAR_READY;
      }
#else
     sonar_readings[sonar_idx] = random(4000,10000);
     sonar_idx++;
     sonar_state = SONAR_READY;
#endif
     break;
    }
  }
  if (sonar_idx == SONAR_NUM) { Timer1.detachInterrupt(); sonar_sweep_active = 0; }
}


void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  if (sonar_sweep_active == 0) {
    for (uint8_t i = 0; i < SONAR_NUM; i++) {
      Serial.print(sonar_readings[i] / US_ROUNDTRIP_CM);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("BUSY");
  }
}

void sonar_sweep_start()
{
 // Serial.println("sonar_sweep_started: " + String(sonar_sweep_t<imer));
  if (sonar_sweep_active == 0) {
    sonar_sweep_active = 1;
    sonar_idx = 0;
    Timer1.attachInterrupt(doSonar);
  }
}

void loop()
{
  if (millis() > sonar_sweep_timer) {
      // This does a periodic sweep of the sonar sensors and prints
      // the values to the serial line.  Default period I am using
      // is 33 ms which should yield about 30 sweeps per second
      oneSensorCycle();
      sonar_sweep_start();
      sonar_sweep_timer = millis() + SONAR_SWEEP_TIME;
  }
  // send data only when you receive data:
  if (Serial.available() >= CMDLEN) {
       // read the incoming byte:
       Serial.readBytes((char *)cmd,CMDLEN);
       processCmd();
  }  
}

void
processCmd()
{
   switch (cmd[0]) {
   case 'P':
      sonar_sweep_start();
      break;   
   case 'D':
      oneSensorCycle();
      break;
   default:
       Serial.write(cmd,CMDLEN);
       Serial.println();
   }
}


