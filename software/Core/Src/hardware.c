/*
 * hardware.c
 *
 *  Created on: Jun 30, 2025
 *      Author: universe
 */

#include "main.h"
#include "main_loop.h"
#include "hardware.h"
#include "flash_storage.h"

struct pwm_state
{
   uint8_t pwmOn;
};

typedef struct
{
   GPIO_TypeDef *port;
   uint32_t      set_mask;
   uint32_t      reset_mask;
} gpio_pin_bsrr_t;

typedef struct
{
   GPIO_TypeDef *port;
   uint32_t      pin;
   uint8_t       bit;
} din_map_t;


static struct pwm_state pwmState[2]={{0,},{0}};

/**
  * @brief LPTIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}

/**
  * @brief LPTIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM5_Init(void)
{

  /* USER CODE BEGIN LPTIM5_Init 0 */

  /* USER CODE END LPTIM5_Init 0 */

  /* USER CODE BEGIN LPTIM5_Init 1 */

  /* USER CODE END LPTIM5_Init 1 */
  hlptim5.Instance = LPTIM5;
  hlptim5.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim5.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
  hlptim5.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim5.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim5.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim5.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  if (HAL_LPTIM_Init(&hlptim5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM5_Init 2 */

  /* USER CODE END LPTIM5_Init 2 */

}

const gpio_pin_bsrr_t dout_bsrr[8] =
{
   { DIGI_OUT_0_GPIO_Port, DIGI_OUT_0_Pin, (uint32_t)DIGI_OUT_0_Pin<<16 },
   { DIGI_OUT_1_GPIO_Port, DIGI_OUT_1_Pin, (uint32_t)DIGI_OUT_1_Pin<<16 },
   { DIGI_OUT_2_GPIO_Port, DIGI_OUT_2_Pin, (uint32_t)DIGI_OUT_2_Pin<<16 },
   { DIGI_OUT_3_GPIO_Port, DIGI_OUT_3_Pin, (uint32_t)DIGI_OUT_3_Pin<<16 },
   { DIGI_OUT_4_GPIO_Port, DIGI_OUT_4_Pin, (uint32_t)DIGI_OUT_4_Pin<<16 },
   { DIGI_OUT_5_GPIO_Port, DIGI_OUT_5_Pin, (uint32_t)DIGI_OUT_5_Pin<<16 },
   { DIGI_OUT_6_GPIO_Port, DIGI_OUT_6_Pin, (uint32_t)DIGI_OUT_6_Pin<<16 },
   { DIGI_OUT_7_GPIO_Port, DIGI_OUT_7_Pin, (uint32_t)DIGI_OUT_7_Pin<<16 }
};

void hw_gpio_set(const uint_fast8_t dout)
{
   if (!pwmState[0].pwmOn)
   {
      dout_bsrr[0].port->BSRR = (dout & 0x01) ? dout_bsrr[0].reset_mask : dout_bsrr[0].set_mask;
   }
   if (!pwmState[0].pwmOn)
   {
      dout_bsrr[1].port->BSRR = (dout & 0x02) ? dout_bsrr[1].reset_mask : dout_bsrr[1].set_mask;
   }

   dout_bsrr[2].port->BSRR = (dout & 0x04) ? dout_bsrr[2].set_mask : dout_bsrr[2].reset_mask;
   dout_bsrr[3].port->BSRR = (dout & 0x08) ? dout_bsrr[3].set_mask : dout_bsrr[3].reset_mask;
   dout_bsrr[4].port->BSRR = (dout & 0x10) ? dout_bsrr[4].set_mask : dout_bsrr[4].reset_mask;
   dout_bsrr[5].port->BSRR = (dout & 0x20) ? dout_bsrr[5].set_mask : dout_bsrr[5].reset_mask;
   dout_bsrr[6].port->BSRR = (dout & 0x40) ? dout_bsrr[6].set_mask : dout_bsrr[6].reset_mask;
   dout_bsrr[7].port->BSRR = (dout & 0x80) ? dout_bsrr[7].set_mask : dout_bsrr[7].reset_mask;
}

static const din_map_t din_map[8] =
{
   { DIGI_IN_0_GPIO_Port, DIGI_IN_0_Pin, 0x01 },
   { DIGI_IN_1_GPIO_Port, DIGI_IN_1_Pin, 0x02 },
   { DIGI_IN_2_GPIO_Port, DIGI_IN_2_Pin, 0x04 },
   { DIGI_IN_3_GPIO_Port, DIGI_IN_3_Pin, 0x08 },
   { DIGI_IN_4_GPIO_Port, DIGI_IN_4_Pin, 0x10 },
   { DIGI_IN_5_GPIO_Port, DIGI_IN_5_Pin, 0x20 },
   { DIGI_IN_6_GPIO_Port, DIGI_IN_6_Pin, 0x40 },
   { DIGI_IN_7_GPIO_Port, DIGI_IN_7_Pin, 0x80 }
};

uint_fast8_t hw_gpio_get(void)
{
   uint_fast8_t din=0;

   din |= (din_map[0].port->IDR & din_map[0].pin) ? din_map[0].bit : 0;
   din |= (din_map[1].port->IDR & din_map[1].pin) ? din_map[1].bit : 0;
   din |= (din_map[2].port->IDR & din_map[2].pin) ? din_map[2].bit : 0;
   din |= (din_map[3].port->IDR & din_map[3].pin) ? din_map[3].bit : 0;
   din |= (din_map[4].port->IDR & din_map[4].pin) ? din_map[4].bit : 0;
   din |= (din_map[5].port->IDR & din_map[5].pin) ? din_map[5].bit : 0;
   din |= (din_map[6].port->IDR & din_map[6].pin) ? din_map[6].bit : 0;
   din |= (din_map[7].port->IDR & din_map[7].pin) ? din_map[7].bit : 0;

   return din;
}

void hw_init_digi_out_0(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   /*Configure GPIO pin Output Level */
   HAL_GPIO_WritePin(DIGI_OUT_0_GPIO_Port, DIGI_OUT_0_Pin, GPIO_PIN_SET);

   /*Configure GPIO pin : DIGI_OUT_0_Pin */
   GPIO_InitStruct.Pin = DIGI_OUT_0_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_PULLDOWN;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(DIGI_OUT_0_GPIO_Port, &GPIO_InitStruct);
}


static void hw_init_pwm_out_0(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   /*Configure GPIO pin : DIGI_OUT_0_Pin */
   GPIO_InitStruct.Pin = DIGI_OUT_0_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF3_LPTIM5;
   HAL_GPIO_Init(DIGI_OUT_0_GPIO_Port, &GPIO_InitStruct);
}


void hw_init_digi_out_1_6_7(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   /*Configure GPIO pins : DIGI_OUT_6_Pin DIGI_OUT_7_Pin DIGI_OUT_1_Pin */
   GPIO_InitStruct.Pin = DIGI_OUT_6_Pin|DIGI_OUT_7_Pin|DIGI_OUT_1_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_PULLDOWN;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}


static void hw_init_pwm_out_1(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   /*Configure GPIO pin Output Level */
   HAL_GPIO_WritePin(GPIOD, DIGI_OUT_1_Pin, GPIO_PIN_RESET);

   /*Configure GPIO pin : DIGI_OUT_0_Pin */
   GPIO_InitStruct.Pin = DIGI_OUT_1_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF1_LPTIM1;
   HAL_GPIO_Init(DIGI_OUT_1_GPIO_Port, &GPIO_InitStruct);
}


#define LPTIM_MAX_CNT    0xFFFF       /* 16-bit counter maximum */


/**
 * @brief  Configure and start LPTIM PWM output (frequency + pulse width in µs).
 *         Chooses the prescaler with smallest frequency error. If the computed
 *         period is larger than the counter range it will be capped to 65535.
 *
 * @param  hlptim       Pointer to LPTIM handle (e.g., &hlptim5)
 * @param  lptim_clk_hz Input clock frequency of the LPTIM in Hz (e.g. 64000000)
 * @param  freq_hz      Desired PWM frequency in Hz
 * @param  pulse_us     Desired pulse width in microseconds
 * @retval HAL status
 */
HAL_StatusTypeDef LPTIM_SetupPWM(LPTIM_HandleTypeDef *hlptim,const uint32_t lptim_clk_hz,const uint32_t freq_hz,const uint32_t pulse_us)
{
   const uint32_t LPTIM_PrescalerTable[] =
   {
      LPTIM_PRESCALER_DIV1,
      LPTIM_PRESCALER_DIV2,
      LPTIM_PRESCALER_DIV4,
      LPTIM_PRESCALER_DIV8,
      LPTIM_PRESCALER_DIV16,
      LPTIM_PRESCALER_DIV32,
      LPTIM_PRESCALER_DIV64,
   };

   uint32_t i;
   float    best_error = 1e9F;
   uint32_t best_period = 0;
   uint32_t best_pulse = 0;
   uint32_t best_idx = 0;

   /* basic checks */
   if(hlptim == NULL || freq_hz == 0 || lptim_clk_hz == 0)
      return HAL_ERROR;

   /* iterate all prescaler candidates */
   for(i = 0; i < sizeof(LPTIM_PrescalerTable)/sizeof(LPTIM_PrescalerTable[0]); i++)
   {
      uint32_t divisor = (1u << i);               /* 1,2,4,...,128 */
      const float timer_clk_d = (float)lptim_clk_hz / (float)divisor;

      const float desired_period_plus1_d = timer_clk_d / (float)freq_hz;
      if(desired_period_plus1_d < 1.0F)
         continue;

      uint64_t desired_period_plus1 = (uint64_t)(desired_period_plus1_d + 0.5F);

      uint32_t candidate_period;
      if(desired_period_plus1 - 1ULL > (uint64_t)LPTIM_MAX_CNT)
         candidate_period = LPTIM_MAX_CNT;
      else
         candidate_period = (uint32_t)(desired_period_plus1 - 1ULL);

      if(candidate_period == 0)
         continue;

      const float real_freq = timer_clk_d / ((float)candidate_period + 1.0);
      const float rel_error = fabs((real_freq - (float)freq_hz) / (float)freq_hz);

      if(rel_error < best_error)
      {
         best_error = rel_error;
         best_period = candidate_period;
         best_idx = i;
      }
   }

   /* if no candidate found → fallback to DIV32 */
   if(best_error == 1e9F)
   {
      best_idx = 5; /* DIV32 */
      uint32_t divisor = (1u << best_idx);
      const float timer_clk_d = (float)lptim_clk_hz / (float)divisor;
      uint64_t desired_period_plus1 = (uint64_t)(timer_clk_d / (float)freq_hz + 0.5F);

      if(desired_period_plus1 == 0)
         return HAL_ERROR;

      if(desired_period_plus1 - 1ULL > (uint64_t)LPTIM_MAX_CNT)
         best_period = LPTIM_MAX_CNT;
      else
         best_period = (uint32_t)(desired_period_plus1 - 1ULL);
   }

   /* compute pulse in ticks using chosen prescaler */
   {
      uint32_t divisor = (1u << best_idx);
      const float timer_clk_d = (float)lptim_clk_hz / (float)divisor;
      const float tick_us = 1e6F / timer_clk_d;
      uint64_t pulse_ticks = (uint64_t)((float)pulse_us / tick_us + 0.5F);

      if(pulse_ticks > (uint64_t)LPTIM_MAX_CNT)
         best_pulse = LPTIM_MAX_CNT;
      else
         best_pulse = (uint32_t)pulse_ticks;

      if(best_pulse >= best_period)
      {
         if(best_period > 0)
            best_pulse = best_period - 1;
         else
            best_pulse = 0;
      }
   }

   /* apply prescaler (STM32H7 → CFGR register) */
   MODIFY_REG(hlptim->Instance->CFGR, LPTIM_CFGR_PRESC, LPTIM_PrescalerTable[best_idx]);

   /* start PWM */
   return HAL_LPTIM_PWM_Start(hlptim, best_period, best_pulse);
}



HAL_StatusTypeDef hw_pwm_set(const uint8_t out,const uint32_t freq, const uint32_t pulse)
{
   // base-clock: 2 MHz
   switch (out)
   {
   case 0:
      if (freq>0)
      {
         if (!pwmState[0].pwmOn)
         {
            hw_init_pwm_out_0();
            MX_LPTIM5_Init();
            HAL_InitTick(TICK_INT_PRIORITY);
            pwmState[0].pwmOn=1;
         }
         return LPTIM_SetupPWM(&hlptim5, 64000000UL,freq,pulse);
      }
      else
      {
         if (pwmState[0].pwmOn)
         {
            hw_init_digi_out_0();
            HAL_InitTick(TICK_INT_PRIORITY);
            pwmState[0].pwmOn=0;
         }
         hw_gpio_set(globalState.digiOut);
         return HAL_OK;
      }
   case 1:
      if (freq>0)
      {
         if (!pwmState[1].pwmOn)
         {
            hw_init_pwm_out_1();
            MX_LPTIM1_Init();
            HAL_InitTick(TICK_INT_PRIORITY);
            pwmState[1].pwmOn=1;
         }
         return LPTIM_SetupPWM(&hlptim1,64000000UL,freq,pulse);
      }
      else
      {
         if (pwmState[1].pwmOn)
         {
            hw_init_digi_out_1_6_7();
            HAL_InitTick(TICK_INT_PRIORITY);
            pwmState[1].pwmOn=0;
         }
         hw_gpio_set(globalState.digiOut);
         return HAL_OK;
      }
   default:
      return HAL_ERROR;
   }
}


extern int32_t hw_get_analogue(const int8_t port)
{
   int32_t val=-1;

   if (testMode==2)
   {
      if ((globalState.ain0state>48) && (globalState.ain0state<2000) && (globalConfig.ana0offs<500))
      {
         globalConfig.ana0offs+=globalState.ain0state/32;
      }
      else if ((globalState.ain0state>62000) && (globalState.ain0state<65480))
      {
         globalConfig.ana0fac+=1;
      }
      if ((globalState.ain1state>48) && (globalState.ain1state<2000) && (globalConfig.ana1offs<500))
      {
         globalConfig.ana1offs+=globalState.ain1state/32;
      }
      else if ((globalState.ain1state>62000) && (globalState.ain1state<65480))
      {
         globalConfig.ana1fac+=1;
      }
   }

   switch (port)
   {
      case 0:
         if (HAL_ADC_PollForConversion(&hadc1,0)==HAL_OK)
         {
            static int adjCtrL=0,adjCtrH=0;

            val=HAL_ADC_GetValue(&hadc1)-globalConfig.ana0offs;
            if (val<0)
            {
               if (globalConfig.isDefaultConfig==0)
               {
                  adjCtrL++;
                  if ((globalConfig.ana0offs>0) && (adjCtrL>50))
                  {
                     adjCtrL=0;
                     globalConfig.ana0offs--;
                  }
               }
               if (testMode==2)
               {
                  globalConfig.ana0offs--;
               }
               val=0;
            }
            else adjCtrL=0;

            val=val*(globalConfig.ana0fac/ANA_CALIB_FACTOR);
            if (val>65535)
            {
               if (globalConfig.isDefaultConfig==0)
               {
                  adjCtrH++;

                  if ((globalConfig.ana0fac>100) && (adjCtrH>50))
                  {
                     adjCtrH=0;
                     globalConfig.ana0fac--;
                  }
               }
               if (testMode==2)
               {
                  globalConfig.ana0fac--;
               }
               val=65535;
            }
            else adjCtrH=0;
         }
         else HAL_ADC_Start(&hadc1);
         return val;
      case 1:
         if (HAL_ADC_PollForConversion(&hadc2,0)==HAL_OK)
         {
            static int adjCtrL=0,adjCtrH=0;

            val=HAL_ADC_GetValue(&hadc2)-globalConfig.ana1offs;
            if (val<0)
            {
               if (globalConfig.isDefaultConfig==0)
               {
                  adjCtrL++;
                  if ((globalConfig.ana1offs>0) && (adjCtrL>50))
                  {
                     adjCtrL=0;
                     globalConfig.ana1offs--;
                  }
               }
               if (testMode==2)
               {
                  globalConfig.ana1offs--;
               }
               val=0;
            }
            else adjCtrL=0;

            val=val*(globalConfig.ana1fac/ANA_CALIB_FACTOR);
            if (val>65535)
            {
               if (globalConfig.isDefaultConfig==0)
               {
                  adjCtrH++;
                  if ((globalConfig.ana1fac>100) && (adjCtrH>50))
                  {
                     adjCtrH=0;
                     globalConfig.ana1fac--;
                  }
               }
               if (testMode==2)
               {
                  globalConfig.ana1fac--;
               }
               val=65535;
            }
            else adjCtrH=0;
         }
         else HAL_ADC_Start(&hadc2);
         return val;
      default:
         return -1;
   }
}


#define ANA_CALIB_STEPS 30000

int32_t hw_calibrate_analogue(const int32_t step,const char *cmd)
{
   int32_t i,val=0;

// TODO: store calibration values and use it in main-loop

   if (step==10)
   {
      for(i=0; i<ANA_CALIB_STEPS;)
      {
         if (HAL_ADC_PollForConversion(&hadc1,0)==HAL_OK)
         {
            val+=HAL_ADC_GetValue(&hadc1);
            i++;
         }
         else HAL_ADC_Start(&hadc1);
      }
      if (val>ANA_CALIB_STEPS*15000) return -1;
      globalConfig.ana0offs=val/ANA_CALIB_STEPS;
      return globalConfig.ana0offs;
   }
   else if (step==20)
   {
      for(i=0; i<ANA_CALIB_STEPS;)
      {
         if (HAL_ADC_PollForConversion(&hadc2,0)==HAL_OK)
         {
            val+=HAL_ADC_GetValue(&hadc2);
            i++;
         }
         else HAL_ADC_Start(&hadc2);
      }
      if (val>ANA_CALIB_STEPS*15000) return -1;
      globalConfig.ana1offs=val/ANA_CALIB_STEPS;
      return globalConfig.ana1offs;
   }
   else if (step==11)
   {
      for(i=0; i<ANA_CALIB_STEPS;)
      {
         if (HAL_ADC_PollForConversion(&hadc1,0)==HAL_OK)
         {
            val+=HAL_ADC_GetValue(&hadc1)-globalConfig.ana0offs;
            i++;
         }
         else HAL_ADC_Start(&hadc1);
      }
      if (val<ANA_CALIB_STEPS*50000) return -1;
      const float fac=(65535.0F/((1.0F*val)/ANA_CALIB_STEPS))*5000.0F;
      if (fac>65535.0F) return -1;
      if (fac<=0.0F) return -1;
      globalConfig.ana0fac=fac;
      return globalConfig.ana0fac;
   }
   else if (step==21)
   {
      for(i=0; i<ANA_CALIB_STEPS;)
      {
         if (HAL_ADC_PollForConversion(&hadc2,0)==HAL_OK)
         {
            val+=HAL_ADC_GetValue(&hadc2)-globalConfig.ana1offs;
            i++;
         }
         else HAL_ADC_Start(&hadc2);
      }
      if (val<ANA_CALIB_STEPS*50000) return -1;
      const float fac=(65535.0F/((1.0F*val)/ANA_CALIB_STEPS))*5000.0F;
      if (fac>65535.0F) return -1;
      if (fac<=0.0F) return -1;
      globalConfig.ana1fac=fac;
      return globalConfig.ana1fac;
   }
   else if (step==12)
   {
      globalConfig.ana0offs=atoi(cmd+3);
      return globalConfig.ana0offs;
   }
   else if (step==22)
   {
      globalConfig.ana1offs=atoi(cmd+3);
      return globalConfig.ana1offs;
   }
   else if (step==13)
   {
      globalConfig.ana0fac=atoi(cmd+3);
      return globalConfig.ana0fac;
   }
   else if (step==23)
   {
      globalConfig.ana1fac=atoi(cmd+3);
      return globalConfig.ana1fac;
   }

   return -1;
}

uint8_t hw_write_config(void)
{
   globalConfig.isDefaultConfig=0;
   if (Flash_Storage_Write(&globalConfig,sizeof(struct global_config),CONFIG_STRUCT_ID))
    return 1;
   else
   {
      // try once more just in case there was an erase/write problem
      if (Flash_Storage_Write(&globalConfig,sizeof(struct global_config),CONFIG_STRUCT_ID))
       return 1;
   }
   return 0;
}

