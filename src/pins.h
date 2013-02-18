#ifndef __CB_PINS_H__
#define __CB_PINS_H__

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


#define MOTOR_A_PWM     PIN_D3 
#define MOTOR_A_DIR     PIN_D12
#define MOTOR_A_BRAKE   PIN_D9
#define MOTOR_A_CURRENT PIN_A0

#define MOTOR_B_PWM     PIN_D11 
#define MOTOR_B_DIR     PIN_D13
#define MOTOR_B_BRAKE   PIN_D8
#define MOTOR_B_CURRENT PIN_A1

/* given the above usage it should be possible to 
   use Timer1 for the ping library so that we don't collide with
   Timer2 which is used for generating PWM on 3 and 11 and Timer0
   which is used for mills and delay */

#endif
