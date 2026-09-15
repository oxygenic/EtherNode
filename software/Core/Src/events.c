#include "main_loop.h"
#include "network.h"
#include "events.h"
#include "stepper.h"
#include <string.h>
#include "debug.h"
#include "hardware.h"

#define TELNET_ERR_PARAMS TELNET_ERROR"parameter missing\r\n"
#define TELNET_ERR_VALUE  TELNET_ERROR"parameter invalid\r\n"
#define TELNET_ERR_SYNTAX TELNET_ERROR"syntax error\r\n"

static char *find_space(char *c)
{
   while ((c) && (*c) && (*c!=' ') && (*c!='\t')) c++;
   return c;
}


static char *skip_spaces(char *c)
{
   while ((c) && (*c) && ((*c==' ') || (*c=='\t') || (*c=='\r') || (*c=='\n'))) c++;
   return c;
}


static bool is_digit(const char *c)
{
   return ((*c>='0') && (*c<='9'));
}


static int32_t getEvtSrcValue(const evtType_t evt,const uint32_t timer[MAX_TIM_NUM],const int32_t mpos[MAX_STEP_AXES],const uint32_t mspd,const uint32_t DInnew)
{
   switch (evt)
   {
      case evtUnused:
         return 0;
         break;
      case evtDIn0:
         if ((DInnew & 0x01)==0x01) return 1;
         return 0;
      case evtDIn1:
         if ((DInnew & 0x02)==0x02) return 1;
         return 0;
      case evtDIn2:
         if ((DInnew & 0x04)==0x04) return 1;
         return 0;
      case evtDIn3:
         if ((DInnew & 0x08)==0x08) return 1;
         return 0;
      case evtDIn4:
         if ((DInnew & 0x10)==0x10) return 1;
         return 0;
      case evtDIn5:
         if ((DInnew & 0x20)==0x20) return 1;
         return 0;
      case evtDIn6:
         if ((DInnew & 0x40)==0x40) return 1;
         return 0;
      case evtDIn7:
         if ((DInnew & 0x80)==0x80) return 1;
         return 0;
      case evtAIn0:
         return globalState.ain0state;
      case evtAIn1:
         return globalState.ain1state;
      case evtEnc0Pos:
         return globalState.motfValue;
      case evtEnc0Spd:
         return globalState.motfSpeed;
      case evtEnc0Acc:
         return globalState.motfAccel;
      case evtTim0:
      case evtTim1:
      case evtTim2:
      case evtTim3:
      case evtTim4:
      case evtTim5:
      case evtTim6:
      case evtTim7:
      case evtTim8:
      case evtTim9:
         return timer[evt-evtTim0];
      case evtVar0:
      case evtVar1:
      case evtVar2:
      case evtVar3:
      case evtVar4:
      case evtVar5:
      case evtVar6:
      case evtVar7:
      case evtVar8:
      case evtVar9:
      case evtVar10:
      case evtVar11:
      case evtVar12:
      case evtVar13:
      case evtVar14:
      case evtVar15:
      case evtVar16:
      case evtVar17:
      case evtVar18:
      case evtVar19:
         return globalState.variable[evt-evtVar0];
      case evtMPos0:
      case evtMPos1:
      case evtMPos2:
      case evtMPos3:
      case evtMPos4:
      case evtMPos5:
      case evtMPos6:
         return mpos[evt-evtMPos0];
         break;
      case evtMSpd:
         return mspd;
      default:
         return false;
   }
   return true;
}


static void doValueSet(const trgType_t trg,const int32_t writeVal,const int32_t evt)
{
   switch (trg)
   {
      case trgDOut0:
         if (writeVal==1) globalState.digiOut|=0x01;
         else if (writeVal==0) globalState.digiOut&=~0x01;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut1:
         if (writeVal==1) globalState.digiOut|=0x02;
         else if (writeVal==0) globalState.digiOut&=~0x02;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut2:
         if (writeVal==1) globalState.digiOut|=0x04;
         else if (writeVal==0) globalState.digiOut&=~0x04;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut3:
         if (writeVal==1) globalState.digiOut|=0x08;
         else if (writeVal==0) globalState.digiOut&=~0x08;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut4:
         if (writeVal==1) globalState.digiOut|=0x10;
         else if (writeVal==0) globalState.digiOut&=~0x10;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut5:
         if (writeVal==1) globalState.digiOut|=0x20;
         else if (writeVal==0) globalState.digiOut&=~0x20;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut6:
         if (writeVal==1) globalState.digiOut|=0x40;
         else if (writeVal==0) globalState.digiOut&=~0x40;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut7:
         if (writeVal==1) globalState.digiOut|=0x80;
         else if (writeVal==0) globalState.digiOut&=~0x80;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         globalState.timer[trg-trgTim0]=writeVal;
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         globalState.variable[trg-trgVar0]=writeVal;
         break;
      case trgPWM0Frq:
         globalState.freq0=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM0Pul:
         globalState.pulse0=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM1Frq:
         globalState.freq1=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgPWM1Pul:
         globalState.pulse1=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
         stepper_move(writeVal,trg-trgMPos0);
         break;
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
         stepper_set_position(writeVal,trg-trgSPos0);
         break;
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
         stepper_home(writeVal,trg-trgHPos0);
         break;
      case trgMSpd0:
      case trgMSpd1:
      case trgMSpd2:
      case trgMSpd3:
      case trgMSpd4:
      case trgMSpd5:
      case trgMSpd6:
         globalConfig.stepVmax[trg-trgMSpd0]=writeVal;
         break;
      case trgMAcc0:
      case trgMAcc1:
      case trgMAcc2:
      case trgMAcc3:
      case trgMAcc4:
      case trgMAcc5:
      case trgMAcc6:
         globalConfig.stepA[trg-trgMAcc0]=writeVal;
         break;
      default:
         logpf(true,"Illegal operator = %d in event %d",writeVal,evt);
         break;
   }

}


static void doValuePlus(const trgType_t trg,const int32_t writeVal,const int32_t evt)
{
   switch (trg)
   {
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         globalState.timer[trg-trgTim0]+=writeVal;
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         globalState.variable[trg-trgVar0]+=writeVal;
         break;
      case trgPWM0Frq:
         globalState.freq0+=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM0Pul:
         globalState.pulse0+=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM1Frq:
         globalState.freq1+=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgPWM1Pul:
         globalState.pulse1+=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
         stepper_move(stepper.current_pos[trg-trgMPos0]+writeVal,trg-trgMPos0);
         break;
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
         stepper_set_position(stepper.current_pos[trg-trgSPos0]+writeVal,trg-trgSPos0);
         break;
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
         stepper_home(stepper.current_pos[trg-trgHPos0]+writeVal,trg-trgHPos0);
         break;
      case trgMSpd0:
      case trgMSpd1:
      case trgMSpd2:
      case trgMSpd3:
      case trgMSpd4:
      case trgMSpd5:
      case trgMSpd6:
         globalConfig.stepVmax[trg-trgMSpd0]+=writeVal;
         break;
      case trgMAcc0:
      case trgMAcc1:
      case trgMAcc2:
      case trgMAcc3:
      case trgMAcc4:
      case trgMAcc5:
      case trgMAcc6:
         globalConfig.stepA[trg-trgMAcc0]+=writeVal;
         break;
      default:
         logpf(true,"Illegal operator + %d in event %d",writeVal,evt);
         break;
   }

}


static void doValueMinus(const trgType_t trg,const int32_t writeVal,const int32_t evt)
{
   switch (trg)
   {
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         globalState.timer[trg-trgTim0]-=writeVal;
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         globalState.variable[trg-trgVar0]-=writeVal;
         break;
      case trgPWM0Frq:
         globalState.freq0-=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM0Pul:
         globalState.pulse0-=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM1Frq:
         globalState.freq1-=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgPWM1Pul:
         globalState.pulse1-=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
         stepper_move(stepper.current_pos[trg-trgMPos0]-writeVal,trg-trgMPos0);
         break;
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
         stepper_set_position(stepper.current_pos[trg-trgSPos0]-writeVal,trg-trgSPos0);
         break;
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
         stepper_home(stepper.current_pos[trg-trgHPos0]-writeVal,trg-trgHPos0);
         break;
      case trgMSpd0:
      case trgMSpd1:
      case trgMSpd2:
      case trgMSpd3:
      case trgMSpd4:
      case trgMSpd5:
      case trgMSpd6:
         globalConfig.stepVmax[trg-trgMSpd0]-=writeVal;
         break;
      case trgMAcc0:
      case trgMAcc1:
      case trgMAcc2:
      case trgMAcc3:
      case trgMAcc4:
      case trgMAcc5:
      case trgMAcc6:
         globalConfig.stepA[trg-trgMAcc0]-=writeVal;
         break;
      default:
         logpf(true,"Illegal operator - %d in event %d",writeVal,evt);
         break;
   }

}


static void doValueOR(const trgType_t trg,const int32_t writeVal,const int32_t evt)
{
   switch (trg)
   {
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         globalState.timer[trg-trgTim0]|=writeVal;
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         globalState.variable[trg-trgVar0]|=writeVal;
         break;
      case trgPWM0Frq:
         globalState.freq0|=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM0Pul:
         globalState.pulse0|=writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM1Frq:
         globalState.freq1|=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgPWM1Pul:
         globalState.pulse1|=writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
         stepper_move(stepper.current_pos[trg-trgMPos0]|writeVal,trg-trgMPos0);
         break;
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
         stepper_set_position(stepper.current_pos[trg-trgSPos0]|writeVal,trg-trgSPos0);
         break;
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
         stepper_home(stepper.current_pos[trg-trgHPos0]|writeVal,trg-trgHPos0);
         break;
      default:
         logpf(true,"Illegal operator | %d in event %d",writeVal,evt);
         break;
   }
}


static void doValueNOT(const trgType_t trg,const int32_t writeVal,const int32_t evt)
{
   switch (trg)
   {
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         globalState.timer[trg-trgTim0]&=~writeVal;
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         globalState.variable[trg-trgVar0]&=~writeVal;
         break;
      case trgPWM0Frq:
         globalState.freq0&=~writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM0Pul:
         globalState.pulse0&=~writeVal;
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM1Frq:
         globalState.freq1&=~writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgPWM1Pul:
         globalState.pulse1&=~writeVal;
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
      {
         int32_t pos=stepper.current_pos[trg-trgMPos0];
         pos&=~writeVal;
         stepper_move(pos,trg-trgMPos0);
         break;
      }
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
      {
         int32_t pos=stepper.current_pos[trg-trgSPos0];
         pos&=~writeVal;
         stepper_set_position(pos,trg-trgSPos0);
         break;
      }
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
      {
         int32_t pos=stepper.current_pos[trg-trgHPos0];
         pos&=~writeVal;
         stepper_home(pos,trg-trgHPos0);
         break;
      }
      default:
         logpf(true,"Illegal operator ~ %d in event %d",writeVal,evt);
         break;
   }
}


static void doValueDiv(const trgType_t trg,const int32_t writeVal,const int32_t evt)
{
   float fac;

   switch (globalConfig.events[evt].dval)
   {
      case evtUnused:
      case evtVar0:
      case evtVar1:
      case evtVar2:
      case evtVar3:
      case evtVar4:
      case evtVar5:
      case evtVar6:
      case evtVar7:
      case evtVar8:
      case evtVar9:
      case evtVar10:
      case evtVar11:
      case evtVar12:
      case evtVar13:
      case evtVar14:
      case evtVar15:
      case evtVar16:
      case evtVar17:
      case evtVar18:
      case evtVar19:
         fac=writeVal/1000.0F;
         break;
      default:
         fac=writeVal;
         break;
   }

   switch (trg)
   {
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         globalState.timer[trg-trgTim0]=(uint32_t)(globalState.timer[trg-trgTim0]/fac);
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         globalState.variable[trg-trgVar0]=(int32_t)(globalState.variable[trg-trgVar0]/fac);
         break;
      case trgPWM0Frq:
         globalState.freq0=(uint32_t)(globalState.freq0/fac);
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM0Pul:
         globalState.pulse0=(uint32_t)(globalState.pulse0/fac);
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM1Frq:
         globalState.freq1=(uint32_t)(globalState.freq1/fac);
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgPWM1Pul:
         globalState.pulse1=(uint32_t)(globalState.pulse1/fac);
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
         stepper_move((int32_t)(stepper.current_pos[trg-trgMPos0]/fac),trg-trgMPos0);
         break;
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
         stepper_set_position((int32_t)(stepper.current_pos[trg-trgSPos0]/fac),trg-trgSPos0);
         break;
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
         stepper_home((int32_t)(stepper.current_pos[trg-trgHPos0]/fac),trg-trgHPos0);
         break;
      case trgMSpd0:
      case trgMSpd1:
      case trgMSpd2:
      case trgMSpd3:
      case trgMSpd4:
      case trgMSpd5:
      case trgMSpd6:
         globalConfig.stepVmax[trg-trgMSpd0]/=fac;
         break;
      case trgMAcc0:
      case trgMAcc1:
      case trgMAcc2:
      case trgMAcc3:
      case trgMAcc4:
      case trgMAcc5:
      case trgMAcc6:
         globalConfig.stepA[trg-trgMAcc0]/=fac;
         break;
      default:
         logpf(true,"Illegal operator / %f in event %d",fac,evt);
         break;
   }

}


static void doValueMul(const trgType_t trg,const int32_t writeVal,const int32_t evt)
{
   float fac;

   switch (globalConfig.events[evt].dval)
   {
      case evtUnused:
      case evtVar0:
      case evtVar1:
      case evtVar2:
      case evtVar3:
      case evtVar4:
      case evtVar5:
      case evtVar6:
      case evtVar7:
      case evtVar8:
      case evtVar9:
      case evtVar10:
      case evtVar11:
      case evtVar12:
      case evtVar13:
      case evtVar14:
      case evtVar15:
      case evtVar16:
      case evtVar17:
      case evtVar18:
      case evtVar19:
         fac=writeVal/1000.0F;
         break;
      default:
         fac=writeVal;
         break;
   }

   switch (trg)
   {
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         globalState.timer[trg-trgTim0]=(uint32_t)(globalState.timer[trg-trgTim0]*fac);
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         globalState.variable[trg-trgVar0]=(int32_t)(globalState.variable[trg-trgVar0]*fac);
         break;
      case trgPWM0Frq:
         globalState.freq0=(uint32_t)(globalState.freq0*fac);
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM0Pul:
         globalState.pulse0=(uint32_t)(globalState.pulse0*fac);
         hw_pwm_set(0,globalState.freq0,globalState.pulse0);
         break;
      case trgPWM1Frq:
         globalState.freq1=(uint32_t)(globalState.freq1*fac);
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgPWM1Pul:
         globalState.pulse1=(uint32_t)(globalState.pulse1*fac);
         hw_pwm_set(1,globalState.freq1,globalState.pulse1);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
         stepper_move((int32_t)(stepper.current_pos[trg-trgMPos0]*fac),trg-trgMPos0);
         break;
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
         stepper_set_position((int32_t)(stepper.current_pos[trg-trgSPos0]*fac),trg-trgSPos0);
         break;
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
         stepper_home((int32_t)(stepper.current_pos[trg-trgHPos0]*fac),trg-trgHPos0);
         break;
      case trgMSpd0:
      case trgMSpd1:
      case trgMSpd2:
      case trgMSpd3:
      case trgMSpd4:
      case trgMSpd5:
      case trgMSpd6:
         globalConfig.stepVmax[trg-trgMSpd0]*=fac;
         break;
      case trgMAcc0:
      case trgMAcc1:
      case trgMAcc2:
      case trgMAcc3:
      case trgMAcc4:
      case trgMAcc5:
      case trgMAcc6:
         globalConfig.stepA[trg-trgMAcc0]*=fac;
         break;
      default:
         logpf(true,"Illegal operator * %f in event %d",fac,evt);
         break;
   }

}

static void doValueToggle(const trgType_t trg,const int32_t evt)
{
   switch (trg)
   {
      case trgDOut0:
         globalState.digiOut^=0x01;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut1:
         globalState.digiOut^=0x02;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut2:
         globalState.digiOut^=0x04;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut3:
         globalState.digiOut^=0x08;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut4:
         globalState.digiOut^=0x10;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut5:
         globalState.digiOut^=0x20;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut6:
         globalState.digiOut^=0x40;
         hw_gpio_set(globalState.digiOut);
         break;
      case trgDOut7:
         globalState.digiOut^=0x80;
         hw_gpio_set(globalState.digiOut);
         break;
      default:
         logpf(true,"Illegal operator # in event %d",evt);
         break;
   }

}


static bool catEvtSrc(char *r,const evtType_t evt)
{
   switch (evt)
   {
      case evtUnused:
         return false;
         break;
      case evtDIn0:
      case evtDIn1:
      case evtDIn2:
      case evtDIn3:
      case evtDIn4:
      case evtDIn5:
      case evtDIn6:
      case evtDIn7:
         snprintf(r,DATA_LENGTH,"%sDIn%d ",r,evt-evtDIn0);
         break;
      case evtAIn0:
      case evtAIn1:
         snprintf(r,DATA_LENGTH,"%sAIn%d ",r,evt-evtAIn0);
         break;
      case evtEnc0Pos:
         strncat(r,"Enc0Pos ",DATA_LENGTH);
         break;
      case evtEnc0Spd:
         strncat(r,"Enc0Spd ",DATA_LENGTH);
         break;
      case evtEnc0Acc:
         strncat(r,"Enc0Acc ",DATA_LENGTH);
         break;
      case evtTim0:
      case evtTim1:
      case evtTim2:
      case evtTim3:
      case evtTim4:
      case evtTim5:
      case evtTim6:
      case evtTim7:
      case evtTim8:
      case evtTim9:
         snprintf(r,DATA_LENGTH,"%s Tim%d ",r,evt-evtTim0);
         break;
      case evtVar0:
      case evtVar1:
      case evtVar2:
      case evtVar3:
      case evtVar4:
      case evtVar5:
      case evtVar6:
      case evtVar7:
      case evtVar8:
      case evtVar9:
      case evtVar10:
      case evtVar11:
      case evtVar12:
      case evtVar13:
      case evtVar14:
      case evtVar15:
      case evtVar16:
      case evtVar17:
      case evtVar18:
      case evtVar19:
         snprintf(r,DATA_LENGTH,"%s Tim%d ",r,evt-evtVar0);
         break;
      case evtMPos0:
      case evtMPos1:
      case evtMPos2:
      case evtMPos3:
      case evtMPos4:
      case evtMPos5:
      case evtMPos6:
         snprintf(r,DATA_LENGTH,"%s MPos%d ",r,evt-evtVar0);
         break;
      case evtMSpd:
         strncat(r,"MSpd ",DATA_LENGTH);
         break;
      default:
         return false;
   }
   return true;
}


static uint8_t getEvtSrc(const char *c)
{
   if (strstr(c,"DIn")==c)
   {
      if (!is_digit(c+3)) return evtUnused;
      const int32_t inNum=atoi(c+3);
      if ((inNum<0) || (inNum>7)) return evtUnused;
      return evtDIn0+inNum;
   }
   else if (strstr(c,"AIn")==c)
   {
      if (!is_digit(c+3)) return evtUnused;
      const int32_t inNum=atoi(c+3);
      if ((inNum<0) || (inNum>2)) return evtUnused;
      return evtAIn0+inNum;
   }
   else if ((strncmp(c,"Enc0Pos",7)==0)) return evtEnc0Pos;
   else if ((strncmp(c,"Enc0Spd",7)==0)) return evtEnc0Spd;
   else if ((strncmp(c,"Enc0Acc",7)==0)) return evtEnc0Acc;
   else if (strstr(c,"Tim")==c)
   {
      if (!is_digit(c+3)) return evtUnused;
      const int32_t inNum=atoi(c+3);
      if ((inNum<0) || (inNum>9)) return evtUnused;
      return evtTim0+inNum;
   }
   else if (strstr(c,"Var")==c)
   {
      if (!is_digit(c+3)) return evtUnused;
      const int32_t inNum=atoi(c+3);
      if ((inNum<0) || (inNum>19)) return evtUnused;
      return evtVar0+inNum;
   }
   else if (strstr(c,"MPos")==c)
   {
      if (!is_digit(c+4)) return evtUnused;
      const int32_t inNum=atoi(c+4);
      if ((inNum<0) || (inNum>6)) return evtUnused;
      return evtMPos0+inNum;
   }
   else if ((strncmp(c,"MSpd",4)==0)) return evtMSpd;
   return evtUnused;
}


static char *getEvtNum(char *c,int32_t *val)
{
   *val=-1;
   if (*c!=' ') return NULL;

   c=skip_spaces(c);
   if (*c==0) return NULL;
   if (!is_digit(c)) return NULL;
   *val=atoi(c);
   if ((*val<0) || (*val>=MAX_EVT_NUM)) return NULL;
   return c;
}


char *handleSetEventCommand(char *cmd)
{
   struct event_def eventDef={evtUnused,cmpNone,trgUnused,opNone,evtUnused,0,evtUnused,0};

   char *c=&cmd[0];
   int32_t evtNum;
   c=getEvtNum(c,&evtNum);
   if (evtNum<0) return TELNET_ERROR"Event number missing\r\n";

   // *** find the source of the event to be configured
   c=find_space(c);
   if (*c==0) return TELNET_ERR_PARAMS;
   c=skip_spaces(c);
   if (*c==0) return TELNET_ERR_PARAMS;

   eventDef.onSrc=getEvtSrc(c);
   if (eventDef.onSrc==evtUnused) return TELNET_ERROR"event source invalid\r\n";

   // *** find the comparison operator of the event to be configured
   c=find_space(c);
   if (*c==0) return TELNET_ERR_PARAMS;
   c=skip_spaces(c);
   if (*c==0) return TELNET_ERR_PARAMS;

   if ((strncmp(c,"< ",2)==0)) eventDef.compOp=cmpLT;
   else if ((strncmp(c,"<=",2)==0)) eventDef.compOp=cmpLE;
   else if ((strncmp(c,">=",2)==0)) eventDef.compOp=cmpGE;
   else if ((strncmp(c,"> ",2)==0)) eventDef.compOp=cmpGT;
   else if ((strncmp(c,"==",2)==0)) eventDef.compOp=cmpEQ;
   else if ((strncmp(c,"!=",2)==0)) eventDef.compOp=cmpNE;
   else return TELNET_ERROR"Comparison operator invalid\r\n";

   // *** find the value to compare with
   c=find_space(c);
   if (*c==0) return TELNET_ERR_PARAMS;
   c=skip_spaces(c);
   if (*c==0) return TELNET_ERR_PARAMS;

   eventDef.dCompareVal=getEvtSrc(c);
   if (eventDef.dCompareVal==evtUnused) eventDef.cCompareVal=atoi(c);

   // *** find the target to write the result to
   c=find_space(c);
   if (*c==0) return TELNET_ERR_PARAMS;
   c=skip_spaces(c);
   if (*c==0) return TELNET_ERR_PARAMS;

   if (strstr(c,"DOut")==c)
   {
      c+=4;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>7)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgDOut0+outNum;
   }
   else if (strstr(c,"Tim")==c)
   {
      c+=3;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>9)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgTim0+outNum;
   }
   else if (strstr(c,"Var")==c)
   {
      c+=3;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>19)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgVar0+outNum;
   }
   else if (strstr(c,"PWM0Frq")==c) eventDef.doTrg=trgPWM0Frq;
   else if (strstr(c,"PWM0Pul")==c) eventDef.doTrg=trgPWM0Pul;
   else if (strstr(c,"PWM1Frq")==c) eventDef.doTrg=trgPWM1Frq;
   else if (strstr(c,"PWM1Pul")==c) eventDef.doTrg=trgPWM1Pul;
   else if (strstr(c,"MPos")==c)
   {
      c+=4;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>6)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgMPos0+outNum;
   }
   else if (strstr(c,"SPos")==c)
   {
      c+=4;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>6)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgSPos0+outNum;
   }
   else if (strstr(c,"HPos")==c)
   {
      c+=4;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>6)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgHPos0+outNum;
   }
   else if (strstr(c,"MSpd")==c)
   {
      c+=4;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>6)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgMSpd0+outNum;
   }
   else if (strstr(c,"MAcc")==c)
   {
      c+=4;
      if (!is_digit(c)) return TELNET_ERR_VALUE;
      const int32_t outNum=atoi(c);
      if ((outNum<0) || (outNum>6)) return TELNET_ERR_VALUE;
      eventDef.doTrg=trgMAcc0+outNum;
   }
   else if (strstr(c,"JMP")==c) eventDef.doTrg=trgJump;
   else return TELNET_ERROR"Result target invalid\r\n";

   // *** find the operator tow rite the result with into the target
   c=find_space(c);
   if (*c==0) return TELNET_ERR_PARAMS;
   c=skip_spaces(c);
   if (*c==0) return TELNET_ERR_PARAMS;

   if (*c=='=') eventDef.withOp=opSet;
   else if (*c=='+') eventDef.withOp=opPlus;
   else if (*c=='-') eventDef.withOp=opMinus;
   else if (*c=='*') eventDef.withOp=opMul;
   else if (*c=='/') eventDef.withOp=opDiv;
   else if (*c=='#') eventDef.withOp=opToggle;
   else if (*c=='|') eventDef.withOp=opOR;
   else if (*c=='!') eventDef.withOp=opNOT;
   else return TELNET_ERROR"Target operator invalid\r\n";

   if (eventDef.withOp!=opToggle)
   {
      //find the dynamic or constant value to write into the target
      c=find_space(c);
      if (*c==0) return TELNET_ERR_PARAMS;
      c=skip_spaces(c);
      if (*c==0) return TELNET_ERR_PARAMS;

      eventDef.dval=getEvtSrc(c);
      if (eventDef.dval==evtUnused)
      {
         if ((eventDef.withOp==opMul) || (eventDef.withOp==opDiv))
         {
            eventDef.cval=(int32_t)(atoff(c)*1000.0F);
         }
         else
         {
            eventDef.cval=atoi(c);
         }
      }
   }

   // snytax checks, reject stuf thatt is not logic in terms of data types to be used
   if ((eventDef.onSrc>=evtDIn0) && (eventDef.onSrc<=evtDIn7))
   {
      // digital inputs can be compared only for being equal or not equal to 0 or 1
      if ((eventDef.compOp!=cmpEQ) && (eventDef.compOp!=cmpNE)) return TELNET_ERR_SYNTAX;
   }
   if ((eventDef.doTrg>=trgDOut0) && (eventDef.doTrg<=trgDOut7))
   {
      // digital outputs can only be toggled or set
      if ((eventDef.withOp!=opSet) && (eventDef.withOp!=opToggle)) return TELNET_ERR_SYNTAX;
   }
   if ((eventDef.doTrg>=trgMSpd0) && (eventDef.doTrg<=trgMSpd6))
   {
      if ((eventDef.withOp==opOR) || (eventDef.withOp==opNOT)) return TELNET_ERR_SYNTAX;
   }
   if ((eventDef.doTrg>=trgMAcc0) && (eventDef.doTrg<=trgMAcc6))
   {
      if ((eventDef.withOp==opOR) || (eventDef.withOp==opNOT)) return TELNET_ERR_SYNTAX;
   }
   if (eventDef.doTrg==trgJump)
   {
      if (eventDef.withOp!=opSet) return TELNET_ERR_SYNTAX;
      if ((eventDef.dval==evtUnused) && (eventDef.cval==evtNum)) return TELNET_ERR_SYNTAX;
   }

   globalConfig.events[evtNum]=eventDef;
   return TELNET_OK"\r\n";
}


char *handleDeleteEventCommand(char *cmd)
{
   int32_t evtNum;
   getEvtNum(cmd,&evtNum);
   if (evtNum<0) return TELNET_ERR_VALUE;

   globalConfig.events[evtNum].onSrc=evtUnused;

   return TELNET_OK"\r\n";
}

#define STR_APPEND(buf, size, fmt, ...)                  \
   do                                                    \
   {                                                     \
      const size_t _l = strlen(buf);                           \
      if (_l < (size))                                  \
      {                                                  \
         const int _n = snprintf((buf) + _l,                   \
                            (size) - _l,                \
                            (fmt), __VA_ARGS__);        \
         if (_n < 0 || (size_t)_n >= (size) - _l)        \
         {                                               \
            /* truncation or error intentionally ignored */ \
         }                                               \
      }                                                  \
   } while (0)



char r[DATA_LENGTH+1]="";

static void createEvtStr(const int32_t evtNum)
{
   if (!catEvtSrc(r,globalConfig.events[evtNum].onSrc)) return;

   if (globalConfig.events[evtNum].compOp==evtUnused) return;

   switch ((evtType_t)globalConfig.events[evtNum].compOp)
   {
      case cmpLT:
         strncat(r,"< ",DATA_LENGTH);
         break;
      case cmpLE:
         strncat(r,"<= ",DATA_LENGTH);
         break;
      case cmpGE:
         strncat(r,">= ",DATA_LENGTH);
         break;
      case cmpGT:
         strncat(r,"> ",DATA_LENGTH);
         break;
      case cmpEQ:
         strncat(r,"== ",DATA_LENGTH);
         break;
      case cmpNE:
         strncat(r,"!= ",DATA_LENGTH);
         break;
      default:
         return;
   }

   if (!catEvtSrc(r,globalConfig.events[evtNum].dCompareVal))
   {
      STR_APPEND(r,DATA_LENGTH,"%d ",(int)globalConfig.events[evtNum].cCompareVal);
   }

   switch ((trgType_t)globalConfig.events[evtNum].doTrg)
   {
      case trgDOut0:
      case trgDOut1:
      case trgDOut2:
      case trgDOut3:
      case trgDOut4:
      case trgDOut5:
      case trgDOut6:
      case trgDOut7:
         STR_APPEND(r,DATA_LENGTH,"DOut%d ",(int)(globalConfig.events[evtNum].doTrg-trgDOut0));
         break;
      case trgTim0:
      case trgTim1:
      case trgTim2:
      case trgTim3:
      case trgTim4:
      case trgTim5:
      case trgTim6:
      case trgTim7:
      case trgTim8:
      case trgTim9:
         STR_APPEND(r,DATA_LENGTH,"Tim%d ",(int)(globalConfig.events[evtNum].doTrg-evtTim0));
         break;
      case trgVar0:
      case trgVar1:
      case trgVar2:
      case trgVar3:
      case trgVar4:
      case trgVar5:
      case trgVar6:
      case trgVar7:
      case trgVar8:
      case trgVar9:
      case trgVar10:
      case trgVar11:
      case trgVar12:
      case trgVar13:
      case trgVar14:
      case trgVar15:
      case trgVar16:
      case trgVar17:
      case trgVar18:
      case trgVar19:
         STR_APPEND(r,DATA_LENGTH,"Var%d ",globalConfig.events[evtNum].doTrg-trgVar0);
         break;
      case trgPWM0Frq:
         strncat(r,"PWM0Frq ",DATA_LENGTH);
         break;
      case trgPWM0Pul:
         strncat(r,"PWM0Pul ",DATA_LENGTH);
         break;
      case trgPWM1Frq:
         strncat(r,"PWM1Frq ",DATA_LENGTH);
         break;
      case trgPWM1Pul:
         strncat(r,"PWM1Pul ",DATA_LENGTH);
         break;
      case trgMPos0:
      case trgMPos1:
      case trgMPos2:
      case trgMPos3:
      case trgMPos4:
      case trgMPos5:
      case trgMPos6:
         STR_APPEND(r,DATA_LENGTH,"MPos%d ",globalConfig.events[evtNum].doTrg-trgMPos0);
         break;
      case trgSPos0:
      case trgSPos1:
      case trgSPos2:
      case trgSPos3:
      case trgSPos4:
      case trgSPos5:
      case trgSPos6:
         STR_APPEND(r,DATA_LENGTH,"SPos%d ",globalConfig.events[evtNum].doTrg-trgSPos0);
         break;
      case trgHPos0:
      case trgHPos1:
      case trgHPos2:
      case trgHPos3:
      case trgHPos4:
      case trgHPos5:
      case trgHPos6:
         STR_APPEND(r,DATA_LENGTH,"HPos%d ",globalConfig.events[evtNum].doTrg-trgHPos0);
         break;
      case trgMSpd0:
      case trgMSpd1:
      case trgMSpd2:
      case trgMSpd3:
      case trgMSpd4:
      case trgMSpd5:
      case trgMSpd6:
         STR_APPEND(r,DATA_LENGTH,"MSpd%d ",globalConfig.events[evtNum].doTrg-trgMSpd0);
         break;
      case trgJump:
         strncat(r,"JMP ",DATA_LENGTH);
         break;
      default:
         return;
   }

   switch ((trgType_t)globalConfig.events[evtNum].withOp)
   {
      case opSet:
         strncat(r,"= ",DATA_LENGTH);
         break;
      case opPlus:
         strncat(r,"+ ",DATA_LENGTH);
         break;
      case opMinus:
         strncat(r,"- ",DATA_LENGTH);
         break;
      case opMul:
         strncat(r,"* ",DATA_LENGTH);
         break;
      case opDiv:
         strncat(r,"/ ",DATA_LENGTH);
         break;
      case opToggle:
         strncat(r,"# ",DATA_LENGTH);
         break;
      case opOR:
         strncat(r,"| ",DATA_LENGTH);
         break;
      case opNOT:
         strncat(r,"~ ",DATA_LENGTH);
         break;
      default:
         return;
   }

   if (globalConfig.events[evtNum].withOp==opToggle) return;

   if (!catEvtSrc(r,globalConfig.events[evtNum].dval))
   {
      STR_APPEND(r,DATA_LENGTH,"%d ",(int)globalConfig.events[evtNum].cval);
   }
}


char *handleListEvent(const int32_t evtNum)
{
   if (globalConfig.events[evtNum].onSrc==evtUnused)
   {
      snprintf(r,DATA_LENGTH,"cdevt %d\r\n",(int)evtNum);
   }
   else
   {
      snprintf(r,DATA_LENGTH,"csevt %d ",(int)evtNum);
      createEvtStr(evtNum);
      strncat(r,"\r\n",DATA_LENGTH);
   }
   return r;
}


char *handleGetEventCommand(char *cmd)
{

   strncpy(r,TELNET_OK,DATA_LENGTH);
   int32_t evtNum;
   getEvtNum(cmd,&evtNum);
   if (evtNum<0) return TELNET_ERR_VALUE;

   createEvtStr(evtNum);
   strncat(r,"\r\n",DATA_LENGTH);

   return r;
}


static void copyState(void *timer,void *mpos,uint32_t *mspd)
{
   memcpy(timer,&globalState.timer[0],sizeof(int32_t)*MAX_TIM_NUM);
   memcpy(mpos,&stepper.current_pos[0],sizeof(int32_t)*MAX_STEP_AXES);
   *mspd=(int)(stepper.v+0.5F);
}


void handleEvents(const uint32_t DInnew)
{
   static int32_t  stepPos=0;
          uint32_t stepCnt=0;
          uint32_t timer[MAX_TIM_NUM];
           int32_t mpos[MAX_STEP_AXES];
          uint32_t mspd=0;
          bool     stateCopied=false;

   for (;;)
   {
      if (globalConfig.events[stepPos].onSrc!=evtUnused)
      {
         int32_t compareVal=0;
         bool condition=false;

         if (!stateCopied)
         {
            copyState(&timer[0],&mpos[0],&mspd);
            stateCopied=true;
         }

         const int32_t onVal=getEvtSrcValue(globalConfig.events[stepPos].onSrc,timer,mpos,mspd,DInnew);

         if (globalConfig.events[stepPos].dCompareVal==evtUnused) compareVal=globalConfig.events[stepPos].cCompareVal;
         else compareVal=getEvtSrcValue(globalConfig.events[stepPos].dCompareVal,timer,mpos,mspd,DInnew);

         switch ((evtType_t)globalConfig.events[stepPos].compOp)
         {
            case cmpLT:
               condition=(onVal<compareVal);
               break;
            case cmpLE:
               condition=(onVal<=compareVal);
               break;
            case cmpGE:
               condition=(onVal>=compareVal);
               break;
            case cmpGT:
               condition=(onVal>compareVal);
               break;
            case cmpEQ:
               condition=(onVal==compareVal);
               break;
            case cmpNE:
               condition=(onVal!=compareVal);
               break;
            default:
               break;
         }
         if (condition) // the condition is fulfilled, so do somethign with somewhat :-D
         {
            int32_t writeVal=0;

            if (globalConfig.events[stepPos].dval==evtUnused) writeVal=globalConfig.events[stepPos].cval;
            else writeVal=getEvtSrcValue(globalConfig.events[stepPos].cval,timer,mpos,mspd,DInnew);

            switch (globalConfig.events[stepPos].withOp)
            {
               case opSet:
                  if ((trgType_t)globalConfig.events[stepPos].doTrg==trgJump)
                  {
                     // set to value minus 1 because it is counted up automatically one step below
                     stepPos=writeVal-1;
                  }
                  else doValueSet((trgType_t)globalConfig.events[stepPos].doTrg,writeVal,stepPos);
                  break;
               case opPlus:
                  doValuePlus((trgType_t)globalConfig.events[stepPos].doTrg,writeVal,stepPos);
                  break;
               case opMinus:
                  doValueMinus((trgType_t)globalConfig.events[stepPos].doTrg,writeVal,stepPos);
                  break;
               case opMul:
                  if (writeVal==0) doValueSet((trgType_t)globalConfig.events[stepPos].doTrg,0,stepPos);
                  else  doValueMul((trgType_t)globalConfig.events[stepPos].doTrg,writeVal,stepPos);
                  break;
               case opDiv:
                  if (writeVal==0) doValueSet((trgType_t)globalConfig.events[stepPos].doTrg,0,stepPos);
                  else  doValueDiv((trgType_t)globalConfig.events[stepPos].doTrg,writeVal,stepPos);
                  break;
               case opToggle:
                  doValueToggle((trgType_t)globalConfig.events[stepPos].doTrg,stepPos);
                  break;
               case opOR:
                  doValueOR((trgType_t)globalConfig.events[stepPos].doTrg,writeVal,stepPos);
                  break;
               case opNOT:
                  doValueNOT((trgType_t)globalConfig.events[stepPos].doTrg,writeVal,stepPos);
                  break;
            }
         }
         stepCnt++;
         if (stepCnt>globalConfig.evtProcNum)
         {
            stepCnt=0;
            return;
         }
      }
      stepPos++;
      if (stepPos>=MAX_EVT_NUM)
      {
         stepPos=0;
         stepCnt=0;
         return;
      }
   }
}
