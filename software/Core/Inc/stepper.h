#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>
#include <stdbool.h>

#include "main.h"

#define MAX_STEP_AXES 7

struct stepper_axes {
   uint32_t currAxis;
   int32_t  dir;
   int32_t  target_pos[MAX_STEP_AXES];
   int32_t  current_pos[MAX_STEP_AXES];
   int32_t  last_current_pos[MAX_STEP_AXES]; // to submit positions via MQTT-J

   float   v;        // aktuelle Geschwindigkeit [steps/s]

   GPIO_TypeDef *stepPort;
   uint16_t      stepPin;
   uint8_t       stepBit;

   enum {ACCEL, CRUISE, DECEL, BRAKE, STOP} state;
   enum {HOME1IN,HOME1OUT,HOME2IN,HOME2OUT,HOMESTOP,HOMEERROR} homing;
   int32_t       homingSteps;
};


extern void stepper_timer_isr();
extern bool stepper_move(const int32_t steps,const uint32_t axis);
extern void stepper_init(void);
extern bool stepper_home(const int32_t steps,const uint32_t axis);
extern void stepper_homing_state(void);
extern bool stepper_stop(void);
extern void stepper_set_position(const int32_t steps,const uint32_t axis);

extern struct stepper_axes stepper;

#endif //STEPPER_H

