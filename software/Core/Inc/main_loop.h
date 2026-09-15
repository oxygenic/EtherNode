#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include "lwip/ip_addr.h"
#include "flash_storage.h"
#include "events.h"
#include "stepper.h"

#define ENABLE_TELNET
#define ENABLE_MODBUS
#define ENABLE_MQTT
#define ENABLE_REST
//define ENABLE_JSON

#ifdef ENABLE_TELNET
 #define CONFIG_FLAG_DISABLE_TELNET    0x0001
#endif
#ifdef ENABLE_MODBUS
 #define CONFIG_FLAG_DISABLE_MODBUS    0x0002
#endif
#ifdef ENABLE_MQTT
 #define CONFIG_FLAG_DISABLE_MQTT      0x0004
#endif
#ifdef ENABLE_REST
 #define CONFIG_FLAG_DISABLE_REST      0x0010
#endif
#define CONFIG_FLAG_TELNET_PUSH_DIGI 0x0010 // special mode where state changes at digital interface are pushed via telnet automatically
#define CONFIG_FLAG_TELNET_PUSH_ANA  0x0020 // special mode where state changes at analogue interface are pushed via telnet automatically
#ifdef ENABLE_MQTT
 #define CONFIG_FLAG_DISABLE_MQTT_JSON 0x0040
#endif
#define CONFIG_FLAG_EVENT_ON_DIN0      0x00080
#define CONFIG_FLAG_EVENT_ON_DIN1      0x00100
#define CONFIG_FLAG_EVENT_ON_DIN2      0x00200
#define CONFIG_FLAG_EVENT_ON_DIN3      0x00400
#define CONFIG_FLAG_EVENT_ON_DIN4      0x00800
#define CONFIG_FLAG_EVENT_ON_DIN5      0x01000
#define CONFIG_FLAG_EVENT_ON_DIN6      0x02000
#define CONFIG_FLAG_EVENT_ON_DIN7      0x04000
#define CONFIG_FLAG_EVENT_ON_DIN_MASK (CONFIG_FLAG_EVENT_ON_DIN0|CONFIG_FLAG_EVENT_ON_DIN1|CONFIG_FLAG_EVENT_ON_DIN2|CONFIG_FLAG_EVENT_ON_DIN3|\
                                       CONFIG_FLAG_EVENT_ON_DIN4|CONFIG_FLAG_EVENT_ON_DIN5|CONFIG_FLAG_EVENT_ON_DIN6|CONFIG_FLAG_EVENT_ON_DIN7)
#define CONFIG_FLAG_EVENT_ON_DIN_SHIFT 7
#define CONFIG_FLAG_ENABLE_QENCP       0x08000
#define CONFIG_FLAG_ENABLE_QENCN       0x10000
#define CONFIG_FLAG_NO_DIN_AT_MQTT     0x20000
#define CONFIG_FLAG_NO_ENC_AT_MQTT     0x40000

#define STATIC_IP_ADDRESS_PORT1         (0xC0A802FD)
#define STATIC_GW_ADDRESS_PORT1         (0xC0A80201)
#define STATIC_NM_ADDRESS_PORT1         (0xFFFFFF00)

#define CONFIG_STRUCT_ID 0

#define MQTT_MAX_STRLEN 24
#define TELNET_PWD_LEN  16

#define DATA_LENGTH 100

#define MAX_EVT_NUM 32

struct __attribute__((packed)) global_config
{
   uint8_t              id; // checks if flash was ever used or contains crap
   uint8_t              digiInit,isDefaultConfig,digiMask;
   ip_addr_t            ip,gw,nm,rwip;
   uint32_t             ana0offs,ana1offs;
   uint16_t             ana0fac,ana1fac;
   uint32_t             flags,res5,res6,res7,res8;
   ip_addr_t            mqttIP;
   uint16_t             mqttPort;
   char                 mqttUser[MQTT_MAX_STRLEN+1],mqttPwd[MQTT_MAX_STRLEN+1],mqttTopic[MQTT_MAX_STRLEN+1];
   char                 telnetPwd[TELNET_PWD_LEN+1];

   ip_addr_t            sntp_ip;
   time_t               sntp_time_offset,sntp_next_update;

   uint32_t             res0,res1,res2,res9,res4,res10;

   struct event_def     events[MAX_EVT_NUM];
   uint16_t             evtInterval; // check event rules every evtInterval msec
   uint8_t              evtProcNum;  // amount of event steps to be proccessed at max per interval
   uint8_t              evtReserved;

   float                stepVmax[MAX_STEP_AXES];
   float                stepA[MAX_STEP_AXES];

   uint8_t              _reserved[0] __attribute__((aligned(FLASHWORD_SIZE)));
};
// needs to be a multiple of FLASHWORD_SIZE 32

struct global_state
{
   uint8_t digiOut;
   uint8_t digiINState;
#ifdef ENABLE_MODBUS
   void   *MODBUSInst;
#endif
   char      mqttMachineID[20];
   uint16_t  ain0state,ain1state; // number of frames cache can accept

   uint32_t  motfValue;
   int32_t   motfSpeed,motfAccel;
   uint64_t  encChange;
   uint32_t  timer[MAX_TIM_NUM];
   int32_t   variable[MAX_VAR_NUM];
   uint32_t  pulse0,freq0,pulse1,freq1;
};

extern struct global_config    globalConfig;
extern struct global_state     globalState;
extern        uint8_t          testMode,evtListCtr,cfgListCtr;
extern        uint32_t         lastMotfValue;
extern        uint64_t         lastDInChange,lastAIn0Change,lastAIn1Change;
extern        char             r[DATA_LENGTH+1];

extern void           main_loop(void);
extern const uint64_t time_ms_get(void);

#endif
