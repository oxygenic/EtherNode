#include <string.h>
#include <stdint.h>

#include "main.h"
#include "main_loop.h"
#include "lwip.h"
#include "network_mqtt.h"
#include "debug.h"
#include "network.h"
#include "hardware.h"
#include "stepper.h"
#if defined(ENABLE_MQTT) || defined(ENABLE_MODBUS)
#include "F:\wxWidgets-3\demos\OpenAPC\libsmartfactory\libsmartfactory.h"
#endif
#include "oapc_libio.h"

#define STATIC_IP_ADDRESS_RW_ACCESS     (0x00000000)

#define STATIC_IP_ADDRESS_MQTT          (0xC0A802FC)

#define IP4_ADDR_INIT(x)   ((ip4_addr_t){ .addr = (x) })

struct global_config globalConfig={CONFIG_STRUCT_ID,
                                   0x00,1,0xFF,
                                   IP4_ADDR_INIT(STATIC_IP_ADDRESS_PORT1),IP4_ADDR_INIT(STATIC_GW_ADDRESS_PORT1),IP4_ADDR_INIT(STATIC_NM_ADDRESS_PORT1),IP4_ADDR_INIT(STATIC_IP_ADDRESS_RW_ACCESS),
                                   450,450,(uint16_t)(ANA_CALIB_FACTOR+1800),(uint16_t)(ANA_CALIB_FACTOR+1800),
                                   CONFIG_FLAG_DISABLE_MQTT|CONFIG_FLAG_DISABLE_MQTT_JSON, // flags
                                   0,0,0,0,
                                   IP4_ADDR_INIT(STATIC_IP_ADDRESS_MQTT),1025,"","","HALnode",
                                   "",
                                   IP4_ADDR_INIT(0),0,0, // SNTP IP, time offset and next update time
                                   0,0,0,0,0,0,
                                   {
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   {evtUnused,cmpNone,evtUnused,0,trgUnused,opNone,evtUnused,0,trgUnused,opNone,evtUnused,0},
                                   },
                                   250, //msec
                                   MAX_EVT_NUM,0, // steps, reserved
                                   {50000.0F,50000.0F,1000.0F,1000.0F,1000.0F,1000.0F,1000.0F},
#ifdef DEBUG
                                   {15000.0F,15000.0F,500.0F,500.0F,500.0F,500.0F,500.0F},
#else
                                   {100000.0F,100000.0F,1100.0F,1100.0F,1100.0F,1100.0F,1100.0F},
#endif
                                   };
struct global_state  globalState={0,0,
   #ifdef ENABLE_MODBUS
                                  NULL,
   #endif
                                  "",
                                  0,0
};
       uint8_t           testMode=0,evtListCtr=MAX_EVT_NUM,cfgListCtr=255;
static int32_t           lastMotfSpeed;
       uint32_t          lastMotfValue=0;
static volatile uint64_t g_time_ms=0;
       uint64_t          lastDInChange=0,lastAIn0Change=0,lastAIn1Change=0;


#ifdef ENABLE_MQTT
static void mqttPublishEncoder(const char *node)
{
   if (lib_mqtt_start(globalConfig.mqttIP,globalConfig.mqttPort,globalState.mqttMachineID,globalConfig.mqttUser,globalConfig.mqttPwd))
   {
      char tmpStr[300+1];
      char tmpVal[70+1];

      snprintf(tmpStr,300,"%s/%s/%s",globalConfig.mqttTopic,globalState.mqttMachineID,node);
      snprintf(tmpVal,70,"{ \"pos\": %u, \"spd\": %d, \"acc\": %d, \"time_ms\": %lu }",(unsigned int)lastMotfValue,(int)globalState.motfSpeed,(int)globalState.motfAccel,(long unsigned int)globalState.encChange);
      lib_mqtt_publish(tmpStr,tmpVal);
   }
}


static void mqttPublishAxis(const char *node)
{
   const uint64_t ts=time_ms_get();
   if (lib_mqtt_start(globalConfig.mqttIP,globalConfig.mqttPort,globalState.mqttMachineID,globalConfig.mqttUser,globalConfig.mqttPwd))
   {
      char tmpStr[300+1];
      char tmpVal[100+1];

      snprintf(tmpStr,300,"%s/%s/%s",globalConfig.mqttTopic,globalState.mqttMachineID,node);
      snprintf(tmpVal,100,"{ \"axis\": %u,  \"pos\": %d, \"spd\": %u, \"time_ms\": %lu }",(unsigned int)stepper.currAxis,(int)stepper.current_pos[stepper.currAxis],(unsigned int)stepper.v,(long unsigned int)ts);
      lib_mqtt_publish(tmpStr,tmpVal);
   }
}


static void mqttPublish(const char *node,const int32_t value,const uint8_t isHex,const int64_t timestamp)
{
   if ((globalConfig.flags & CONFIG_FLAG_DISABLE_MQTT_JSON)==0)
   {
      if (lib_mqtt_start(globalConfig.mqttIP,globalConfig.mqttPort,globalState.mqttMachineID,globalConfig.mqttUser,globalConfig.mqttPwd))
      {
         char tmpStr[300+1];
         char tmpVal[70+1];

         snprintf(tmpStr,300,"%s/%s/%s",globalConfig.mqttTopic,globalState.mqttMachineID,node);
         snprintf(tmpVal,70,"{ \"val\": %u, \"time_ms\": %lu }",(unsigned int)value,(long unsigned int)timestamp);
         lib_mqtt_publish(tmpStr,tmpVal);
      }
   }
   else if ((globalConfig.flags & CONFIG_FLAG_DISABLE_MQTT)==0)
   {
      if (lib_mqtt_start(globalConfig.mqttIP,globalConfig.mqttPort,globalState.mqttMachineID,globalConfig.mqttUser,globalConfig.mqttPwd))
      {
         char tmpStr[300+1];
         char tmpVal[20+1];

         snprintf(tmpStr,300,"%s/%s/%s",globalConfig.mqttTopic,globalState.mqttMachineID,node);
         if (isHex) snprintf(tmpVal,20,"0x%X",(int)value);
         else snprintf(tmpVal,20,"%d",(int)value);
         lib_mqtt_publish(tmpStr,tmpVal);
      }
   }
}
#endif


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   if (htim->Instance==TIM6)
   {
      stepper_timer_isr();
   }
   else if (htim->Instance==TIM5)
   {
      int32_t i;

      g_time_ms++;
      for (i=0; i<MAX_TIM_NUM; i++)
       globalState.timer[i]++;
      //HAL_GPIO_TogglePin(DIGI_OUT_7_GPIO_Port,DIGI_OUT_7_Pin);
   }
   else if(htim->Instance==TIM3)
   {
      const uint32_t current=__HAL_TIM_GET_COUNTER(&htim2);
      lastMotfSpeed=globalState.motfSpeed;
      globalState.motfSpeed=(current-lastMotfValue)*60;
      globalState.motfAccel=abs(globalState.motfSpeed)-abs(lastMotfSpeed);
      lastMotfValue=current;
      //HAL_GPIO_TogglePin(DIGI_OUT_6_GPIO_Port,DIGI_OUT_6_Pin);
   }
}

const uint64_t time_ms_get(void)
{
   __disable_irq();      // Interrupt kurz sperren
   const uint64_t t = g_time_ms;
   __enable_irq();       // Interrupt wieder freigeben
   return t;
}


char *handleListConfig(const int32_t cfgNum)
{
   switch (cfgNum)
   {
      case 0:
         snprintf(r,DATA_LENGTH,"ciout 0x%X\r\n",globalConfig.digiInit);
         return r;
      case 1:
         snprintf(r,DATA_LENGTH,"cimsk 0x%X\r\n",globalConfig.digiMask);
         return r;
      case 10:
      {
         const ip_addr_t tip={ntohl(globalConfig.ip.addr)};

         snprintf(r,DATA_LENGTH,"csip0 %s\r\n",ip4addr_ntoa(&tip));
         return r;
      }
      case 11:
      {
         const ip_addr_t tip={ntohl(globalConfig.gw.addr)};

         snprintf(r,DATA_LENGTH,"csgw0 %s\r\n",ip4addr_ntoa(&tip));
         return r;
      }
      case 12:
      {
         const ip_addr_t tip={ntohl(globalConfig.nm.addr)};

         snprintf(r,DATA_LENGTH,"csnm0 %s\r\n",ip4addr_ntoa(&tip));
         return r;
      }
      case 13:
      {
         const ip_addr_t tip={ntohl(globalConfig.rwip.addr)};

         snprintf(r,DATA_LENGTH,"csipw %s\r\n",ip4addr_ntoa(&tip));
         return r;
      }
      case 14:
      {
         const ip_addr_t tip={ntohl(globalConfig.mqttIP.addr)};

         snprintf(r,DATA_LENGTH,"csipq %s\r\n",ip4addr_ntoa(&tip));
         return r;
      }
      case 15:
         snprintf(r,DATA_LENGTH,"csptq %d\r\n",globalConfig.mqttPort);
         return r;
      case 16:
         if (isWriteAllowed) snprintf(r,DATA_LENGTH,"csusq %s\r\n",globalConfig.mqttUser);
         else strncpy(r,"csusq ********\r\n",DATA_LENGTH);
         return r;
      case 17:
         if (isWriteAllowed) snprintf(r,DATA_LENGTH,"cspwq %s\r\n",globalConfig.mqttPwd);
         else strncpy(r,"cspwq ********\r\n",DATA_LENGTH);
         return r;
      case 18:
         snprintf(r,DATA_LENGTH,"cstpq %s\r\n",globalConfig.mqttTopic);
         return r;
      case 19:
         strncpy(r,"cdflg 2147483648\r\n",DATA_LENGTH);
         return r;
      case 20:
         snprintf(r,DATA_LENGTH,"csflg %u\r\n",(unsigned int)globalConfig.flags);
         return r;
      case 30:
      case 31:
      case 32:
      case 33:
      case 34:
      case 35:
      case 36:
         snprintf(r,DATA_LENGTH,"caxsv %d %d\r\n",(int)(cfgNum-12),(int)globalConfig.stepVmax[cfgNum-30]);
         return r;
      case 40:
      case 41:
      case 42:
      case 43:
      case 44:
      case 45:
      case 46:
         snprintf(r,DATA_LENGTH,"caxsa %d %d\r\n",(int)(cfgNum-19),(int)globalConfig.stepA[cfgNum-40]);
         return r;
      case 50:
         snprintf(r,DATA_LENGTH,"csets %d\r\n",globalConfig.evtInterval);
         return r;
      case 51:
         snprintf(r,DATA_LENGTH,"csepn %d\r\n",globalConfig.evtProcNum);
         return r;

      case 200:
         snprintf(r,DATA_LENGTH,"csa0f %d\r\n",globalConfig.ana0fac);
         return r;
      case 201:
         snprintf(r,DATA_LENGTH,"csa1f %d\r\n",globalConfig.ana1fac);
         return r;
      case 202:
         snprintf(r,DATA_LENGTH,"csa0o %u\r\n",(unsigned int)globalConfig.ana0offs);
         return r;
      case 203:
         snprintf(r,DATA_LENGTH,"csa1o %u\r\n",(unsigned int)globalConfig.ana1offs);
         return r;
      case 254:
         evtListCtr=0;
         break;
   }
   return NULL;
}


void main_loop(void)
{
   uint32_t DInnew=0;
   int32_t  ain0new=0,ain1new=0;
   char     recvBuffer[UART_RECV_BUFFER+1],*sendBuffer=NULL;
   uint32_t uartReceived=0,sendBufferSize=0,sendBufferPos=0;
   uint32_t ain0sum=0,ain1sum=0;
   uint16_t ain0cnt=0,ain1cnt=0;
   uint32_t currMotfValue=0;
   uint64_t lastEvtTime=0;

//   HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
   HAL_InitTick(TICK_INT_PRIORITY);

   uint32_t lastTick=HAL_GetTick();
   uint32_t lastSendTick=HAL_GetTick();
   handleEvents(0); // initial call with all timers set to 0, here initialisations can be done with comparison of TimX == 0
   HAL_TIM_Base_Start_IT(&htim5);
   HAL_TIM_Base_Start_IT(&htim3);
   HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
   for (;;)
   {
//HAL_GPIO_TogglePin(DIGI_OUT_5_GPIO_Port,DIGI_OUT_5_Pin);
      stepper_homing_state();
      MX_LWIP_Process();
      DInnew=(hw_gpio_get() & globalConfig.digiMask);

      const int32_t sendTickDiff=HAL_GetTick()-lastSendTick;

      {
         const uint64_t currTime=time_ms_get();
         bool  eventTriggered=false;

         if (DInnew!=globalState.digiINState)
          lastDInChange=currTime;

         if ((globalConfig.flags & CONFIG_FLAG_EVENT_ON_DIN_MASK)!=0)
         {
            const int32_t evtBit=((globalConfig.flags & CONFIG_FLAG_EVENT_ON_DIN_MASK)>>CONFIG_FLAG_EVENT_ON_DIN_SHIFT);

            if (((globalState.digiINState & evtBit)==0) &&
                ((DInnew & evtBit)!=0)) eventTriggered=true;
         }

         if (eventTriggered)
         {
            handleEvents(DInnew);
         }
         else if ((globalConfig.evtInterval>0) &&
             (currTime>=lastEvtTime+globalConfig.evtInterval))
         {
            lastEvtTime=currTime;
            handleEvents(DInnew);
         }
         else if (cfgListCtr<255)
         {
            char *r=handleListConfig(cfgListCtr);
            if (r) telnet_append_response(r);
            cfgListCtr++;
         }
         else if (evtListCtr<MAX_EVT_NUM)
         {
            char *r=handleListEvent(evtListCtr);
            if (r) telnet_append_response(r);
            evtListCtr++;
            if (evtListCtr>=MAX_EVT_NUM) telnet_append_response(TELNET_PROMPT);
         }
      }

      if ((globalConfig.flags & CONFIG_FLAG_ENABLE_QENCP)==CONFIG_FLAG_ENABLE_QENCP)
      {
         static uint_fast8_t quencStep=0;

         quencStep++;
         switch (quencStep)
         {
            case 1:
               globalState.digiOut|=0x04;
               break;
            case 20:
               globalState.digiOut|=0x08;
               break;
            case 30:
               globalState.digiOut&=~0x04;
               break;
            case 40:
               globalState.digiOut&=~0x08;
               break;
            case 50:
               quencStep=0;
               break;
         }
         hw_gpio_set(globalState.digiOut);
      }

      // currMotfValue is "current" only in context of comparison, the very latest MOTF-value according to the
      // encoder register is stored in lastMotfValue
      if (currMotfValue!=lastMotfValue)
      {
         globalState.encChange=time_ms_get();
         currMotfValue=lastMotfValue;
         globalState.motfValue=currMotfValue;
#ifdef ENABLE_MQTT
         if ((globalConfig.flags & CONFIG_FLAG_NO_ENC_AT_MQTT)==0)
         {
            if ((globalConfig.flags & CONFIG_FLAG_DISABLE_MQTT_JSON)==0)
            {
               mqttPublishEncoder("Enc");
            }
            else
            {
               mqttPublish("EncPos",currMotfValue,0,0);
               mqttPublish("EncSpd",globalState.motfSpeed,0,0);
               mqttPublish("EncAcc",globalState.motfAccel,0,0);
            }
         }
#endif
      }
      if (stepper.last_current_pos[stepper.currAxis]!=stepper.current_pos[stepper.currAxis])
      {
         static uint32_t waitCtr=0;

         waitCtr++;
         if (waitCtr>150)
         {
            waitCtr=0;
            if ((globalConfig.flags & CONFIG_FLAG_DISABLE_MQTT_JSON)==0)
            {
               mqttPublishAxis("Axis");
            }
            else
            {
               if (stepper.currAxis==0)
               {
                  mqttPublish("Axis0Pos",stepper.current_pos[stepper.currAxis],0,0);
                  mqttPublish("Axis0Spd",stepper.v,0,0);
               }
               else if (stepper.currAxis==1)
               {
                  mqttPublish("Axis1Pos",stepper.current_pos[stepper.currAxis],0,0);
                  mqttPublish("Axis1Spd",stepper.v,0,0);
               }
            }
            stepper.last_current_pos[stepper.currAxis]=stepper.current_pos[stepper.currAxis];
         }
      }

      if ((DInnew!=globalState.digiINState) || (sendTickDiff>20000))
      {
         lastSendTick=HAL_GetTick();
         globalState.digiINState=DInnew;
#ifdef ENABLE_MQTT
         if ((globalConfig.flags & CONFIG_FLAG_NO_DIN_AT_MQTT)==0)
         {
            mqttPublish("DIn",DInnew,1,lastDInChange);
         }
#endif
#ifdef ENABLE_TELNET
         if ((globalConfig.flags & CONFIG_FLAG_TELNET_PUSH_DIGI)==CONFIG_FLAG_TELNET_PUSH_DIGI)
         {
            char cmd[20+1];

            snprintf(cmd,20,"cginp 0x%X\r\n",(int)DInnew);
            telnet_append_response(cmd);
         }
#endif
      }


      // TODO: take analogue calibration values into account
      ain0new=hw_get_analogue(0);
      if (ain0new>=0)
      {
         ain0cnt++;
         ain0sum+=ain0new;
      }
      // TODO: take analogue calibration values into account
      ain1new=hw_get_analogue(1);
      if (ain1new>=0)
      {
         ain1cnt++;
         ain1sum+=ain1new;
      }

      const int32_t tickDiff=HAL_GetTick()-lastTick;
      if ((tickDiff>500) || (ain0cnt>10000) || (ain1cnt>10000) || (sendTickDiff>20000))
      {
         lastTick=HAL_GetTick();
         if (ain0cnt>0)
         {
            ain0new=ain0sum/ain0cnt;
            if (ain0new!=globalState.ain0state)
             lastAIn0Change=time_ms_get();
            if ((ain0new>=0) && (ain0cnt>100) && (abs(ain0new-globalState.ain0state)>8))
            {
               globalState.ain0state=ain0new;
               ain0cnt=0;
               ain0sum=0;
#ifdef ENABLE_MQTT
               mqttPublish("AIn0",ain0new,0,lastAIn0Change);
#endif
#ifdef ENABLE_TELNET
               if ((globalConfig.flags & CONFIG_FLAG_TELNET_PUSH_ANA)==CONFIG_FLAG_TELNET_PUSH_ANA)
               {
                  char cmd[20+1];

                  snprintf(cmd,20,"cgana0 %d\r\n",(int)ain0new);
                  telnet_append_response(cmd);
               }
#endif
            }
         }
         if (ain1cnt>0)
         {
            ain1new=ain1sum/ain1cnt;
            if (ain1new!=globalState.ain1state)
             lastAIn1Change=time_ms_get();
            if ((ain1new>=0) && (ain1cnt>100) && (abs(ain1new-globalState.ain1state)>8))
            {
               globalState.ain1state=ain1new;
               ain1cnt=0;
               ain1sum=0;
#ifdef ENABLE_MQTT
               mqttPublish("AIn1",ain1new,0,lastAIn0Change);
#endif
#ifdef ENABLE_TELNET
               if ((globalConfig.flags & CONFIG_FLAG_TELNET_PUSH_ANA)==CONFIG_FLAG_TELNET_PUSH_ANA)
               {
                  char cmd[20+1];

                  snprintf(cmd,20,"cgana1 %d\r\n",(int)ain1new);
                  telnet_append_response(cmd);
               }
#endif
            }
         }
      }

      if ((sendBuffer) && (sendBufferPos<sendBufferSize))
      {
         if (HAL_UART_Transmit(&huart1,(uint8_t*)&sendBuffer[sendBufferPos],1,1)==HAL_OK)
          sendBufferPos++;
      }
      else if (HAL_UART_Receive(&huart1,(uint8_t*)(recvBuffer+uartReceived),1,1)==HAL_OK)
      {
         uartReceived++;
         if (uartReceived>=UART_RECV_BUFFER)
         {
            logpf(true,"UART RX overflow");
            uartReceived=0;
         }
         recvBuffer[uartReceived]=0;
         if ((strstr(recvBuffer,"\r")) || (strstr(recvBuffer,"\n")))
         {
            sendBuffer=handleASCIICommand(recvBuffer,NULL);
            sendBufferSize=strlen(sendBuffer);
            sendBufferPos=0;
            uartReceived=0;
         }
      }

      if (testMode)
      {
         static int32_t stepCtr=100000;
         static uint8_t lastDIn=0;
                char    cmd[50+1];

         if (lastDIn!=DInnew)
         {
            lastDIn=DInnew;
            snprintf(cmd,50,"0x%X\r\n",(unsigned int)DInnew);
            telnet_append_response(cmd);
         }

         stepCtr++;
         if (stepCtr>200)
         {
            static uint8_t dOut=0x01;

            stepCtr=0;
            hw_gpio_set(dOut);
            dOut=dOut<<1;
            if (testMode==2)
            {
               if ((dOut==0x04) && (globalConfig.isDefaultConfig!=0))
               {
                  snprintf(cmd,50,"Offs: %u - %u\r\n",(unsigned int)globalConfig.ana0offs,(unsigned int)globalConfig.ana1offs);
                  telnet_append_response(cmd);
               }
               if ((dOut==0x40) && (globalConfig.isDefaultConfig!=0))
               {
                  snprintf(cmd,50,"Fact: %u - %u\r\n",globalConfig.ana0fac,globalConfig.ana1fac);
                  telnet_append_response(cmd);
               }
               if (dOut==0x00)
               {
                  dOut=0x01;
                  snprintf(cmd,50,"Val:  %u - %u\r\n",globalState.ain0state,globalState.ain1state);
                  telnet_append_response(cmd);
                  if (((globalState.ain0state>2) && (globalState.ain1state>2) &&
                       (globalState.ain0state<48) && (globalState.ain1state<48)) ||

                        ((globalState.ain0state>65480) && (globalState.ain1state>65480) &&
                         (globalState.ain0state<65530) && (globalState.ain1state<65530)))
                  {
                     telnet_append_response("Calibration done:\r\n");
                     snprintf(cmd,50,"Offs: %u - %u\r\n",(unsigned int)globalConfig.ana0offs,(unsigned int)globalConfig.ana1offs);
                     telnet_append_response(cmd);
                     snprintf(cmd,50,"Fact: %u - %u\r\n",globalConfig.ana0fac,globalConfig.ana1fac);
                     telnet_append_response(cmd);
                     telnet_append_response("e>");
                     testMode=0;
                  }
               }
            }
            else if (dOut==0x00)
            {
               dOut=0x01;
            }
         }
      }

   }

}

