#include "stepper.h"
#include "main.h"
#include "main_loop.h"
#include "hardware.h"

struct stepper_axes stepper={0,1,
                             {0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0},
                             0.0F,
                             NULL,0,0,
                             STOP,
                             HOMESTOP,
                             0
                            };


static uint32_t timerTickHz=9821428;


void stepper_timer_isr()
{
    if (stepper.state == STOP) {
        HAL_TIM_Base_Stop_IT(&htim6);
        return;
    }

    // STEP-Puls
    HAL_GPIO_TogglePin(stepper.stepPort,stepper.stepPin);
    globalState.digiOut^=stepper.stepBit;
    stepper.current_pos[stepper.currAxis]+=stepper.dir;

    int32_t remaining = stepper.target_pos[stepper.currAxis] - stepper.current_pos[stepper.currAxis];
    const float steps_to_stop=(stepper.v*stepper.v)/(2.0F*globalConfig.stepA[stepper.currAxis]);

    // ⚠️ Bremsbedingung hat Vorrang
    if (stepper.state!=DECEL)
    {
       switch (stepper.homing)
       {
          case HOME1IN:
          case HOME2IN:
             //reference switch was hit, so decelerate and stop
             if (HAL_GPIO_ReadPin(DIGI_IN_7_GPIO_Port,DIGI_IN_7_Pin)==GPIO_PIN_SET)
             {
                stepper.state=DECEL;
                stepper.target_pos[stepper.currAxis]=stepper.current_pos[stepper.currAxis]+(steps_to_stop*stepper.dir);
             }
             break;
          case HOME1OUT:
          case HOME2OUT:
             //reference switch was left, so decelerate and stop
             if (HAL_GPIO_ReadPin(DIGI_IN_7_GPIO_Port,DIGI_IN_7_Pin)==GPIO_PIN_RESET)
             {
                stepper.state=DECEL;
                stepper.target_pos[stepper.currAxis]=stepper.current_pos[stepper.currAxis]+(steps_to_stop*stepper.dir);
             }
             break;
          default:
             break;
       }
       if (abs(remaining)<=steps_to_stop)
       {
          stepper.state = DECEL;
       }
    }

    switch (stepper.state)
    {
       case ACCEL:
          stepper.v = sqrtf(stepper.v * stepper.v + 2.0f *globalConfig.stepA[stepper.currAxis]);
          if (stepper.v >=globalConfig.stepVmax[stepper.currAxis])
          {
             stepper.v =globalConfig.stepVmax[stepper.currAxis];
             stepper.state = CRUISE;
          }
          break;
       case CRUISE:
          // nichts tun
       default:
          break;
       case DECEL:
       {
          const float under_sqrt = stepper.v*stepper.v - 2.0f *globalConfig.stepA[stepper.currAxis];
          if (under_sqrt>0.0f)
          {
             stepper.v=sqrtf(under_sqrt);
          }
          if (stepper.v <= 0.0f || abs(remaining)==0)
          {
             stepper.v = 0.0f;
             stepper.state = STOP;
             return;
          }
          break;
       }
       case BRAKE:
       {
          const float under_sqrt = stepper.v*stepper.v - 2.0f *globalConfig.stepA[stepper.currAxis];
          if (under_sqrt>0.0f)
          {
             stepper.v=sqrtf(under_sqrt);
          }
          else
          {
             stepper.v=0.0f;
             stepper.target_pos[stepper.currAxis]=stepper.current_pos[stepper.currAxis];
             stepper.state=STOP;
             return;
          }
          break;
       }
    }


    uint32_t arr = (uint32_t)(timerTickHz / stepper.v/2); // divide by two because each ISR-call is only halve of a full period of the target step frequency
    if (arr < 42) arr = 42;
    __HAL_TIM_SET_AUTORELOAD(&htim6, arr);
}


void stepper_init(void)
{
   uint32_t tim_clk;

   const uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
   const uint32_t apb1_presc =(RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos;

   /* Prescaler encoding: 0..3 = DIV1, >=4 = DIV2..DIV16 */
   if (apb1_presc >= 4) tim_clk = pclk1 * 2;
   else tim_clk = pclk1;

   timerTickHz=tim_clk / (htim6.Instance->PSC + 1);
}


// move an axis without any logic and exlcusion checks
static bool raw_stepper_move(const int32_t steps,const uint32_t axis)
{
   switch (axis)
   {
      case 0:
         stepper.stepPort=DIGI_OUT_0_GPIO_Port;
         stepper.stepPin=DIGI_OUT_0_Pin;
         stepper.stepBit=0x01;
         break;
      case 1:
         stepper.stepPort=DIGI_OUT_1_GPIO_Port;
         stepper.stepPin=DIGI_OUT_1_Pin;
         stepper.stepBit=0x02;
         break;
      case 2:
         stepper.stepPort=DIGI_OUT_2_GPIO_Port;
         stepper.stepPin=DIGI_OUT_2_Pin;
         stepper.stepBit=0x04;
         break;
      case 3:
         stepper.stepPort=DIGI_OUT_3_GPIO_Port;
         stepper.stepPin=DIGI_OUT_3_Pin;
         stepper.stepBit=0x08;
         break;
      case 4:
         stepper.stepPort=DIGI_OUT_4_GPIO_Port;
         stepper.stepPin=DIGI_OUT_4_Pin;
         stepper.stepBit=0x10;
         break;
      case 5:
         stepper.stepPort=DIGI_OUT_5_GPIO_Port;
         stepper.stepPin=DIGI_OUT_5_Pin;
         stepper.stepBit=0x20;
         break;
      case 6:
         stepper.stepPort=DIGI_OUT_6_GPIO_Port;
         stepper.stepPin=DIGI_OUT_6_Pin;
         stepper.stepBit=0x40;
         break;
      default:
         return false;
   }
   stepper.currAxis=axis;

   stepper.target_pos[stepper.currAxis] = stepper.current_pos[stepper.currAxis] + steps;

   stepper.v = 10.0f;  // Startgeschwindigkeit > 0!
   stepper.state = ACCEL;

   if (steps<0)
   {
      globalState.digiOut|=0x80;
      hw_gpio_set(globalState.digiOut);
      stepper.dir=-1;
   }
   else
   {
      globalState.digiOut&=~0x80;
      hw_gpio_set(globalState.digiOut);
      stepper.dir=1;
   }


   HAL_TIM_Base_Start_IT(&htim6);
   return true;
}


void stepper_homing_state(void)
{
   if (stepper.homing==HOMESTOP) return; // not in referencing mode, so return here
   if (stepper.state!=STOP) return; // we're referencing but axes are still moving, so nothing to do yet
   switch (stepper.homing)
   {
      case HOME1IN:
         // leave reference switch at full speed
         stepper.homing=HOME1OUT;
         if (!raw_stepper_move(-stepper.homingSteps,stepper.currAxis)) stepper.homing=HOMEERROR;
         break;
      case HOME1OUT:
         //re-enter homing switch at halve speed
         stepper.homing=HOME2IN;
         globalConfig.stepVmax[stepper.currAxis]/=2.0F;
         if (!raw_stepper_move(stepper.homingSteps,stepper.currAxis)) stepper.homing=HOMEERROR;
         break;
      case HOME2IN:
         //leave homing switch at quarter of initial speed
         stepper.homing=HOME2OUT;
         globalConfig.stepVmax[stepper.currAxis]/=2.0F;
         if (!raw_stepper_move(-stepper.homingSteps,stepper.currAxis)) stepper.homing=HOMEERROR;
         break;
      case HOME2OUT:
         stepper.homing=HOMESTOP;
         stepper_set_position(stepper.currAxis,0);
         break;
      default:
         // should never happen
         break;
   }
}


void stepper_set_position(const int32_t steps,const uint32_t axis)
{
   stepper.current_pos[stepper.currAxis]=steps;
   stepper.last_current_pos[stepper.currAxis]=steps;
   stepper.target_pos[stepper.currAxis]=steps;
}


bool stepper_stop(void)
{
   if ((stepper.state==CRUISE) || (stepper.state==ACCEL) || (stepper.state==DECEL))
   {
      stepper.state=BRAKE;
      return true;
   }
   return false;
}


bool stepper_move(const int32_t steps,const uint32_t axis)
{
   if (stepper.state!=STOP) return false;
   if (stepper.homing!=HOMESTOP) return false;
   if (globalConfig.stepVmax[axis]<10) return false;
   if (globalConfig.stepA[axis]<10) return false;
   return raw_stepper_move(steps,axis);
}


bool stepper_home(const int32_t steps,const uint32_t axis)
{
   if (stepper.state!=STOP) return false;
   if ((stepper.homing!=HOMESTOP) && (stepper.homing!=HOMEERROR)) return false;
   stepper.homing=HOME1IN;
   stepper.homingSteps=steps;
   return raw_stepper_move(steps,axis);
}
