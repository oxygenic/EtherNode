#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

#include "main.h"
#include "main_loop.h"
#include "network.h"
#include "debug.h"
#include "buffer.h"
#include "hardware.h"
#include "flash_storage.h"
#include "stepper.h"

#ifdef ENABLE_TELNET

static struct tcp_pcb *currentPcb=NULL;

#define HTTPD_MAX_RETRIES                   10

#define NET1_BUFFER_LENGTH (1460)


struct network1_state
{
   char          sendBuffer[NET1_BUFFER_LENGTH+1];
   uint_fast16_t sendBufferLength;
   uint_fast16_t sentLength;
   char          recvBuffer[NET1_BUFFER_LENGTH+100];
   char         *recvBufferPtr;
   uint_fast16_t recvBufferLength;
   uint_fast8_t  retries;
};

static err_t           network1_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static struct          network1_state telnetState;
       char            telnetBuffer[TELNET_BUF_MAX+1];
       bool            isWriteAllowed=false;


static void network1_checkTelnet(struct tcp_pcb *pcb, struct network1_state *hs);

static void network_err(void *arg, err_t err)
{
   currentPcb=NULL;
}

static void network1_close_conn(struct tcp_pcb *pcb, struct network1_state *hs)
{
   hs=hs;

   if (!pcb) return;
   logpf(false,"network1_close_conn");
   tcp_recv(pcb, NULL);
   const err_t err = tcp_close(pcb);
   if (err != ERR_OK) tcp_abort(pcb);
   currentPcb=NULL;
}


static err_t network1_send_data(struct tcp_pcb *pcb, struct network1_state *hs)
{
   err_t err=ERR_BUF;
   uint_fast16_t len;
   u16_t snd_buf;

   if (hs == NULL)
   {
      // Already closed or nothing more to send; be robust: close
      logpf(false,"network1_send_data");
      network1_close_conn(pcb, hs);
      return ERR_CONN;
   }

   if (hs->sentLength>=hs->sendBufferLength) return ERR_OK; // everything already sent

   // We cannot send more data than space available in the send buffer.
   snd_buf = tcp_sndbuf(pcb);
   len = LWIP_MIN(snd_buf,hs->sendBufferLength-hs->sentLength);
   len = LWIP_MIN(len, TCP_MSS);  // Segmentierung auf MSS

   if (len>0) do
   {
      err = tcp_write(pcb, hs->sendBuffer+hs->sentLength,(u16_t)len,TCP_WRITE_FLAG_COPY);
      if (err == ERR_MEM)
      {
         tcp_output(pcb); // flush when outbuffer is full
         len /= 2;
      }
   } 
   while ((err == ERR_MEM) && (len > 1));
   tcp_output(pcb);

   if (err == ERR_OK)
   {
      hs->sentLength+=len;
      if (hs->sentLength>=hs->sendBufferLength) // buffer sent completely so reset it
      {
         hs->sendBuffer[0]=0;
         hs->sendBufferLength=0;
         hs->sentLength=0;
      }
   }
   return err;
}



#if (LWIP_TIMERS)
static err_t network1_poll(void *arg, struct tcp_pcb *pcb)
{
   struct network1_state *hs = arg;
  
   if (hs == NULL) 
   {
      if (pcb->state == ESTABLISHED) 
      {
         // arg is null, close
         logpf(false,"network1_poll: arg is NULL, close");
         network1_close_conn(pcb, hs);
         return ERR_ABRT;
      }
   } 
   else if (hs->sendBufferLength>0)
   {
#ifndef ENV_FWTEST
      hs->retries++;
      if (hs->retries == HTTPD_MAX_RETRIES)
      {
         logpf(false,"network1_poll: too many retries, close");
         network1_close_conn(pcb, hs);
         return ERR_ABRT;
      }
#endif
      network1_send_data(pcb, hs);
   }
   return ERR_OK;
}
#endif


static err_t network1_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p_in, err_t err)
{
   struct network1_state *hs = arg;

   if ((pcb!=currentPcb) || (!p_in) || (!pcb))
   {
      logpf(false,"closing connection on recv");
      network1_close_conn(pcb, hs);
      return ERR_OK;
   }

   if (hs->recvBufferLength+p_in->len<=NET1_BUFFER_LENGTH)
   {
      signed char *first=(signed char*)p_in->payload;

      if ((*first)!=-1) // no negotiation data; this check is a bit poor, normally only values <0 should be filtered, all other should be accepted
      {         
         memcpy(&hs->recvBuffer[hs->recvBufferLength],p_in->payload,p_in->len);
         hs->recvBufferLength+=p_in->len;
         hs->recvBufferPtr=hs->recvBuffer;
      }
      tcp_recved(pcb,p_in->len);      
   }

   if (hs == NULL)
   {
      // be robust
      logpf(true,"network1_recv: hs is NULL, abort");
      network1_close_conn(pcb, hs);
      return ERR_OK;
   }


   if (hs->recvBufferLength>0)
    network1_checkTelnet(pcb,hs);

   if ((err != ERR_OK) || (!p_in))
   {
      if (p_in) pbuf_free(p_in);
      logpf(false,"Error, network1_recv: err=%d or p_in=%p, abort",err,p_in);
      network1_close_conn(pcb, hs);
      return ERR_OK;
   }
   pbuf_free(p_in);
   return ERR_OK;
}



static void network1_append_response(struct tcp_pcb *pcb, struct network1_state *hs,const char *in_response)
{
   static uint8_t locked=0; // poor lock for function as it may be called out of two contexts...
          err_t   ret=ERR_OK;

   while (locked);
   locked=1;
   if (!in_response) return;
   if (strlen(in_response)+telnetState.sendBufferLength>=NET1_BUFFER_LENGTH)
   {
      logpf(true,"telnet send buffer full");
      ret=network1_send_data(pcb,hs);
   }
   else
   {
      strncat(telnetState.sendBuffer,in_response,NET1_BUFFER_LENGTH);
      telnetState.sendBufferLength+=(u16_t)strlen(in_response);
      if (strstr(telnetState.sendBuffer,"\r")) ret=network1_send_data(pcb,hs);
   }
   if (ret==ERR_CONN)
    network1_close_conn(pcb,hs);
   locked=0;
}


void telnet_append_response(const char *in_response)
{
   if (((globalConfig.flags & CONFIG_FLAG_DISABLE_TELNET)==0) && (currentPcb))
    network1_append_response(currentPcb,&telnetState,in_response);
   else
   {
      telnetState.sendBuffer[0]=0;
      telnetState.sendBufferLength=0;
      telnetState.sentLength=0;
   }
}


err_t network1_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
   struct network1_state *hs = arg;

   LWIP_UNUSED_ARG(len);

   if (hs == NULL) return ERR_OK;

   hs->retries=0;

   network1_send_data(pcb, hs);
   return ERR_OK;
}

static err_t network1_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
   err=err;

   if (currentPcb) network1_close_conn(currentPcb,NULL);
   currentPcb=pcb;

   logpf(false,"Accepting incoming Telnet connection");

   // Decrease the listen backlog counter
#ifdef ENV_FWTEST
#pragma warning (disable: 4127)
#endif
   tcp_accepted(((struct tcp_pcb_listen*)arg));
#ifdef ENV_FWTEST
#pragma warning (default: 4127)
#endif
   tcp_setprio(pcb,TCP_PRIO_NORMAL);

   // Initialize the structure
   memset(&telnetState,0,sizeof(struct network1_state));
   telnetState.retries = 0;
   telnetState.sentLength=0xFFFF;

   // Tell TCP that this is the structure we wish to be passed for our callbacks
   tcp_arg(pcb,&telnetState);

   // Set up the various callback functions
   tcp_recv(pcb, network1_recv);
   tcp_err(pcb, network_err);
#if (LWIP_TIMERS)
   tcp_poll(pcb, network1_poll,2);
#endif
   tcp_sent(pcb, network1_sent);

   telnetState.sentLength=0;
   network1_append_response(pcb,&telnetState,"HALnode Command Interface\r\n"TELNET_PROMPT);
   return ERR_OK;
}


void network1_init(void)
{
   struct tcp_pcb *pcb;
   err_t err;

   pcb = tcp_new();
   if (!pcb)
   {
      logpf(1,"tcp_new1() failed!");
      return;
   }
   pcb->so_options |= SOF_KEEPALIVE;   // Keepalive aktivieren
   pcb->keep_idle=20000;             // 20s Inaktivität → erstes KA
   pcb->keep_intvl=4000;             // alle 4s erneut
   pcb->keep_cnt=4;                  // nach 4 erfolglosen Versuchen → Abbruch
   err=tcp_bind(pcb, IP_ADDR_ANY,23);
   if (err!=ERR_OK)
   {
      logpf(true,"failed binding port 23");
      return;
   }
   pcb = tcp_listen(pcb);
   tcp_arg(pcb, pcb);
   tcp_accept(pcb, network1_accept);
}


static void stripFileName(char *cmd)
{
   while ((strlen(cmd)>0) &&
          ((cmd[strlen(cmd)-1]=='\r') || (cmd[strlen(cmd)-1]=='\n')))
    cmd[strlen(cmd)-1]=0;
}


static char *skip_spaces(char *c)
{
   while ((*c) && ((*c==' ') || (*c=='\t'))) c++;
   return c;
}


static char *skip_number(char *c)
{
   while ((*c) &&
          (((*c>='0') && (*c<='9')) ||
            (*c=='-') || (*c=='+') || (*c == '.'))) c++;
   return c;
}


static char *handleAxisCommand(const char *cmd,const struct tcp_pcb *pcb)
{
   if (strstr(cmd,"st")==cmd) // "stop movement"
   {
      if (stepper_stop())
       return TELNET_OK"\r\n";
      else
       return TELNET_ERROR"\r\n";
   }

   if (stepper.state!=STOP) return TELNET_ERROR"\r\n";
   if (strlen(cmd)<6) return TELNET_ERROR"\r\n";
   const uint32_t axis=atoi(cmd+3);
   if (axis>=MAX_STEP_AXES) return TELNET_ERROR"\r\n";

   if (isWriteAllowed)
   {
      const int32_t value=atoi(cmd+5);

      if (strstr(cmd,"sp")==cmd) // "set position" - set current axis position
      {
         stepper_set_position(value,axis);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"sv")==cmd) // "set velocity" - set axis speed
      {
         if (value==0) stepper_stop();
         else globalConfig.stepVmax[axis]=value;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"sa")==cmd) // "set acceleration" - set axis acceleration
      {
         globalConfig.stepA[axis]=abs(value);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"mp")==cmd) // "move position" - move axis to position
      {
         if (stepper_move(value,axis)) return TELNET_OK"\r\n";
         return TELNET_ERROR"\r\n";
      }
      else if (strstr(cmd,"hp")==cmd) // "home position" - reference axis with given maximum move distance
      {
         if (abs(value)<1000) return TELNET_ERROR"Distance too short \r\n";
         if (globalConfig.stepVmax[axis]<1000) return TELNET_ERROR"Velocity too low\r\n";
         if (stepper_home(value,axis)) return TELNET_OK"\r\n";
         return TELNET_ERROR"\r\n";
      }
   }
   if (strstr(cmd,"gp")==cmd) // get current axis position
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",(int)stepper.current_pos[axis]);
      return(telnetBuffer);
   }
   else if (strstr(cmd,"hs")==cmd) // get homing state
   {
      if (stepper.homing==HOMEERROR) return TELNET_ERROR"Referencing failed\r\n";
      else if (stepper.homing==HOMESTOP) return TELNET_OK"Not referencing\r\n";
      else return TELNET_OK"Referencing in progress\r\n";
   }
   else if (strstr(cmd,"gv")==cmd) // "get velocity" - get axis speed
   {
      uint32_t spd;
      if (axis==stepper.currAxis)
      {
         spd=stepper.v;
         if ((spd==0) && (stepper.homing!=HOMESTOP) && (stepper.homing!=HOMEERROR))
          spd=1;
      }
      else spd=0;
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%u\r\n",(unsigned int)spd);
      return(telnetBuffer);
   }
   else if (strstr(cmd,"ga")==cmd) // "get acceleration" - get axis accel value
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%u\r\n",(unsigned int)globalConfig.stepA[axis]);
      return(telnetBuffer);
   }

   return TELNET_ERROR"\r\n";
}


static char *doHandleASCIICommand(char *cmd,const struct tcp_pcb *pcb)
{
   if ((!cmd) || (cmd[0]==0)) return NULL;
   logpf(false,"Command: %s",cmd);
   if ((!pcb) || // no pcb, so we're coming from serial port
       (globalConfig.rwip.addr==0) || (globalConfig.rwip.addr==pcb->remote_ip.addr)) // only the configured IP is allowed to write, or all of them when none is configured
   {
      isWriteAllowed=true;
   }
   else
   {
      isWriteAllowed=false;
   }

   if (isWriteAllowed)
   {
      if (strstr(cmd,"csout 0x")==cmd)
      {
         globalState.digiOut=(int)strtol(cmd+8,NULL,16);
         hw_gpio_set(globalState.digiOut);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csfrq")==cmd)
      {
         const uint8_t freqOut=atoi(cmd+6);

         if ((freqOut>=0) && (freqOut<=1))
         {
            const char *c=cmd+8;
            const uint32_t freq=atoi(c); //Hz
            c=skip_number((char*)c);
            c=skip_spaces((char*)c);
            if ((c) && (freq<=500000))
            {
               const uint32_t pulse=atoi(c); //usec

               if (hw_pwm_set(freqOut,freq,pulse)==HAL_OK)
                return TELNET_OK"\r\n";
            }
         }
         return TELNET_ERROR"\r\n";
      }
      else if (strstr(cmd,"ccana")==cmd)
      {
         const int mode=atoi(cmd+6);
         if (mode==30)
         {
            testMode=2;
            return TELNET_OK"\r\n";
         }
         else
         {
            int32_t ret=hw_calibrate_analogue(mode,cmd+6);
            if (ret<0) return TELNET_ERROR"\r\n";
            snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%ld\r\n",ret);
            return(telnetBuffer);
         }
      }
      else if (strstr(cmd,"ciout 0x")==cmd)
      {
         globalConfig.digiInit=(int)strtol(cmd+8,NULL,16);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"cimsk 0x")==cmd)
      {
         globalConfig.digiMask=(int)strtol(cmd+8,NULL,16);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csip0")==cmd)
      {
         ip4_addr_t newip;
         if (ip4addr_aton(cmd+6,&newip))
         {
            globalConfig.ip.addr=htonl(newip.addr);
            return TELNET_OK"\r\n";
         }
         return TELNET_ERROR"\r\n";
      }
      else if (strstr(cmd,"csgw0")==cmd)
      {
         ip4_addr_t newip;
         if (ip4addr_aton(cmd+6,&newip))
         {
            globalConfig.gw.addr=htonl(newip.addr);
            return TELNET_OK"\r\n";
         }
         return TELNET_ERROR"\r\n";
      }
      else if (strstr(cmd,"csnm0")==cmd)
      {
         ip4_addr_t newip;
         if (ip4addr_aton(cmd+6,&newip))
         {
            globalConfig.nm.addr=htonl(newip.addr);
            return TELNET_OK"\r\n";
         }
         return TELNET_ERROR"\r\n";
      }
      else if (strstr(cmd,"csipw")==cmd)
      {
         ip4_addr_t newip;
         if (ip4addr_aton(cmd+6,&newip))
         {
            globalConfig.rwip=newip;
            return TELNET_OK"\r\n";
         }
         return TELNET_ERROR"\r\n";
      }
#ifdef ENABLE_MQTT
      else if (strstr(cmd,"csipq")==cmd)
      {
         ip4_addr_t newip;
      if (ip4addr_aton(cmd+6,&newip))
      {
         globalConfig.mqttIP.addr=htonl(newip.addr);
         return TELNET_OK"\r\n";
      }
      return TELNET_ERROR"\r\n";
      }
      else if (strstr(cmd,"csptq")==cmd)
      {
         const int32_t val=atoi(cmd+6);
         if ((val<1) || (val>65535)) return TELNET_ERROR"\r\n";
         globalConfig.mqttPort=val;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csusq")==cmd)
      {
         snprintf(globalConfig.mqttUser,MQTT_MAX_STRLEN,cmd+6);
         globalConfig.mqttUser[MQTT_MAX_STRLEN]=0;
         stripFileName(globalConfig.mqttUser);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"cspwq")==cmd)
      {
         snprintf(globalConfig.mqttPwd,MQTT_MAX_STRLEN,cmd+6);
         globalConfig.mqttPwd[MQTT_MAX_STRLEN]=0;
         stripFileName(globalConfig.mqttPwd);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"cstpq")==cmd)
      {
         snprintf(globalConfig.mqttTopic,MQTT_MAX_STRLEN,cmd+6);
         globalConfig.mqttTopic[MQTT_MAX_STRLEN]=0;
         stripFileName(globalConfig.mqttTopic);
         return TELNET_OK"\r\n";
      }
#endif
      else if (strstr(cmd,"cscfg")==cmd)
      {
         const uint8_t enable=atoi(cmd+8);

         if (enable>1) return TELNET_ERROR"\r\n";

         switch (cmd[6])
         {
#ifdef ENABLE_TELNET
            case 't':
               if (enable) globalConfig.flags&=~CONFIG_FLAG_DISABLE_TELNET;
               else globalConfig.flags|=CONFIG_FLAG_DISABLE_TELNET;
               break;
            case 'a':
               if (enable) globalConfig.flags|=CONFIG_FLAG_TELNET_PUSH_ANA;
               else globalConfig.flags&=~CONFIG_FLAG_TELNET_PUSH_ANA;
               break;
            case 'd':
               if (enable) globalConfig.flags|=CONFIG_FLAG_TELNET_PUSH_DIGI;
               else globalConfig.flags&=~CONFIG_FLAG_TELNET_PUSH_DIGI;
               break;
#endif
#ifdef ENABLE_MODBUS
            case 'm':
               if (enable) globalConfig.flags&=~CONFIG_FLAG_DISABLE_MODBUS;
               else globalConfig.flags|=CONFIG_FLAG_DISABLE_MODBUS;
               break;
#endif
#ifdef ENABLE_MQTT
            case 'q':
               if (enable)
               {
                  globalConfig.flags&=~CONFIG_FLAG_DISABLE_MQTT;
                  globalConfig.flags|=CONFIG_FLAG_DISABLE_MQTT_JSON;
               }
              else
              {
                  globalConfig.flags|=CONFIG_FLAG_DISABLE_MQTT;
               }
               break;
            case 'Q':
               if (enable)
               {
                  globalConfig.flags&=~CONFIG_FLAG_DISABLE_MQTT_JSON;
                  globalConfig.flags|=CONFIG_FLAG_DISABLE_MQTT;
               }
               else
               {
                  globalConfig.flags|=CONFIG_FLAG_DISABLE_MQTT_JSON;
               }
               break;
#endif
#ifdef ENABLE_REST
            case 'r':
               if (enable) globalConfig.flags&=~CONFIG_FLAG_DISABLE_REST;
               else globalConfig.flags|=CONFIG_FLAG_DISABLE_REST;
               break;
#endif
            default:
               return TELNET_ERROR"\r\n";
         }
         return TELNET_OK"\r\n";
      }

      else if (strstr(cmd,"clcfg")==cmd)
      {
         cfgListCtr=0;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"cwcfg")==cmd)
      {
         if (hw_write_config())
          return TELNET_OK"\r\n";
         else
          return TELNET_ERROR"\r\n";
      }
      else if (strstr(cmd,"csevt")==cmd)
      {
         return handleSetEventCommand(cmd+5);
      }
      else if (strstr(cmd,"csets")==cmd) // set event processing time slot
      {
         int32_t num=atoi(cmd+6);
         if (num!=0) // a value of 0 turns of time triggered events completely
         {
            if (num<50) num=50;
            else if (num>65000) num=65000;
         }
         globalConfig.evtInterval=num;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csein")==cmd) // set event processing input bit
      {
         const int32_t bit=1<<(atoi(cmd+6))<<CONFIG_FLAG_EVENT_ON_DIN_SHIFT;
         globalConfig.flags|=bit;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"cdein")==cmd) // set event processing input bit
      {
         const int32_t bit=1<<(atoi(cmd+6))<<CONFIG_FLAG_EVENT_ON_DIN_SHIFT;
         globalConfig.flags&=~bit;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csepn")==cmd) // event processing number per time slot
      {
         int32_t num=atoi(cmd+6);
         if (num<5) num=5;
         else if (num>250) num=250;
         globalConfig.evtProcNum=num;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"cstim")==cmd)
      {
         if (cmd[5]!=' ') return TELNET_ERROR"\r\n";

         int32_t num,val;
         sscanf(cmd,"cstim %d %d",(int*)&num,(int*)&val);
         if ((num<0) || (num>=MAX_TIM_NUM))  return TELNET_ERROR"\r\n";
         globalState.timer[num]=val;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csvar")==cmd)
      {
         if (cmd[5]!=' ') return TELNET_ERROR"\r\n";

         int32_t num,val;
         sscanf(cmd,"csvar %d %d",(int*)&num,(int*)&val);
         if ((num<0) || (num>=MAX_VAR_NUM))  return TELNET_ERROR"\r\n";
         globalState.variable[num]=val;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csflg")==cmd)
      {
         if (cmd[5]!=' ') return TELNET_ERROR"\r\n";

         const uint32_t flag=atoi(cmd+6);
         globalConfig.flags|=flag;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"cdflg")==cmd)
      {
         if (cmd[5]!=' ') return TELNET_ERROR"\r\n";

         const uint32_t flag=atoi(cmd+6);
         globalConfig.flags&=~flag;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csa0f")==cmd)
      {
         const int32_t val=atoi(cmd+6);
         if ((val<1) || (val>65535)) return TELNET_ERROR"\r\n";
         globalConfig.ana0fac=val;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csa1f")==cmd)
      {
         const int32_t val=atoi(cmd+6);
         if ((val<1) || (val>65535)) return TELNET_ERROR"\r\n";
         globalConfig.ana1fac=val;
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csa0o")==cmd)
      {
         globalConfig.ana0offs=atoi(cmd+6);
         return TELNET_OK"\r\n";
      }
      else if (strstr(cmd,"csa1o")==cmd)
      {
         globalConfig.ana0offs=atoi(cmd+6);
         return TELNET_OK"\r\n";
      }
      // end of authorised IP only section
   }
   if (strstr(cmd,"cginp")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"0x%X\r\n",globalState.digiINState);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgenp")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%u\r\n",(unsigned int)lastMotfValue);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgens")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",(int)globalState.motfSpeed);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgena")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",(int)globalState.motfAccel);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgana")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d %d\r\n",globalState.ain0state,globalState.ain1state);
      return(telnetBuffer);
   }
   else if (strstr(cmd,"cglog")==cmd)
   {
      if (ufb_empty(&lFrameBuffer)) snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"no more logs available\r\n");
      else
      {
         struct uframe *logFrame;

         logFrame=ufb_front(&lFrameBuffer);
         snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",(const char*)logFrame->d.rawblock);
         ufb_pop(&lFrameBuffer);
      }
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgerr")==cmd)
   {
      if (logGetErr()[0]==0) snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"no error\r\n");
      else snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",logGetErr());
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgip0")==cmd)
   {
      const ip_addr_t tip={ntohl(globalConfig.ip.addr)};

      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",ip4addr_ntoa(&tip));
      return telnetBuffer;
   }
   else if (strstr(cmd,"cggw0")==cmd)
   {
      const ip_addr_t tip={ntohl(globalConfig.gw.addr)};

      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",ip4addr_ntoa(&tip));
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgnm0")==cmd)
   {
      const ip_addr_t tip={ntohl(globalConfig.nm.addr)};

      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",ip4addr_ntoa(&tip));
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgipw")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",ip4addr_ntoa(&globalConfig.rwip));
      return telnetBuffer;
   }

#ifdef ENABLE_MQTT
   else if (strstr(cmd,"cgipq")==cmd)
   {
      const ip_addr_t tip={ntohl(globalConfig.mqttIP.addr)};

      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",ip4addr_ntoa(&tip));
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgptq")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",globalConfig.mqttPort);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgusq")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",globalConfig.mqttUser);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgpwq")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",globalConfig.mqttPwd);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgtpq")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%s\r\n",globalConfig.mqttTopic);
      return telnetBuffer;
   }
#endif // MQTT
   else if (strstr(cmd,"cgcfg")==cmd)
   {
      strncpy(telnetBuffer,TELNET_OK,TELNET_BUF_MAX);
      if ((globalConfig.flags & CONFIG_FLAG_DISABLE_TELNET)==0)
      {
         if ((globalConfig.flags & (CONFIG_FLAG_TELNET_PUSH_ANA|CONFIG_FLAG_TELNET_PUSH_DIGI))!=0)
          strncat(telnetBuffer,"TNet push(t a/d) ",TELNET_BUF_MAX);
         else
          strncat(telnetBuffer,"TNet(t) ",TELNET_BUF_MAX);
      }
#ifdef CONFIG_FLAG_DISABLE_MODBUS
      if ((globalConfig.flags & CONFIG_FLAG_DISABLE_MODBUS)==0)
       strncat(telnetBuffer,"MODBUS(m) ",TELNET_BUF_MAX);
#endif
#ifdef CONFIG_FLAG_DISABLE_MQTT
      if ((globalConfig.flags & CONFIG_FLAG_DISABLE_MQTT)==0)
       strncat(telnetBuffer,"MQTT-T(q) ",TELNET_BUF_MAX);
      if ((globalConfig.flags & CONFIG_FLAG_DISABLE_MQTT_JSON)==0)
       strncat(telnetBuffer,"MQTT-J(Q) ",TELNET_BUF_MAX);
#endif
#ifdef CONFIG_FLAG_DISABLE_REST
      if ((globalConfig.flags & CONFIG_FLAG_DISABLE_REST)==0)
       strncat(telnetBuffer,"HTTP/REST(r) ",TELNET_BUF_MAX);
#endif
      strncat(telnetBuffer,"\r\n",TELNET_BUF_MAX);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cshal")==cmd) // enable E1X0XX-specific configurations
   {
      globalConfig.flags|=CONFIG_FLAG_TELNET_PUSH_DIGI;
      globalConfig.flags&=~CONFIG_FLAG_TELNET_PUSH_ANA;
      globalConfig.flags|=CONFIG_FLAG_DISABLE_MODBUS;
      globalConfig.flags|=CONFIG_FLAG_DISABLE_MQTT;
      globalConfig.flags|=CONFIG_FLAG_DISABLE_REST;
      return TELNET_OK"\r\n";
   }

   else if (strstr(cmd,"crrrr")==cmd)
   {
      NVIC_SystemReset();
   }
   else if (strstr(cmd,"cvers")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"v%02d\r\n",10);
      return telnetBuffer;
   }
   else if (strstr(cmd,"ctest")==cmd)
   {
      if (testMode==0) testMode=1;
      else testMode=0;
      return TELNET_OK"\r\n";
   }
   else if (strstr(cmd,"cgevt")==cmd)
   {
      return handleGetEventCommand(cmd+5);
   }
   else if (strstr(cmd,"cdevt")==cmd)
   {
      return handleDeleteEventCommand(cmd+5);
   }
   else if (strstr(cmd,"clevt")==cmd)
   {
      evtListCtr=0;
      return TELNET_OK"\r\n";
   }
   else if (strstr(cmd,"cgets")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",globalConfig.evtInterval);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgepn")==cmd)
   {
      snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",globalConfig.evtProcNum);
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgtim")==cmd)
   {
      if (cmd[5]==' ')
      {
         const int32_t num=atoi(cmd+6);
         if ((num>=0) && (num<MAX_TIM_NUM))
         {
            snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",(int)globalState.timer[num]);
            return telnetBuffer;
         }
         return TELNET_ERROR"\r\n";
      }
      else
      {
         snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d %d %d %d %d %d %d %d %d %d\r\n",
                  (int)globalState.timer[0],(int)globalState.timer[1],(int)globalState.timer[2],(int)globalState.timer[3],(int)globalState.timer[4],
                  (int)globalState.timer[5],(int)globalState.timer[6],(int)globalState.timer[7],(int)globalState.timer[8],(int)globalState.timer[9]);
         return telnetBuffer;
      }
      return telnetBuffer;
   }
   else if (strstr(cmd,"cgvar")==cmd)
   {
      if (cmd[5]==' ')
      {
         const int32_t num=atoi(cmd+6);
         if ((num>=0) && (num<MAX_VAR_NUM))
         {
            snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d\r\n",(int)globalState.variable[num]);
            return telnetBuffer;
         }
         return TELNET_ERROR"\r\n";
      }
      else
      {
         snprintf(telnetBuffer,TELNET_BUF_MAX,TELNET_OK"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",
               (int)globalState.variable[0],(int)globalState.variable[1],(int)globalState.variable[2],(int)globalState.variable[3],(int)globalState.variable[4],
               (int)globalState.variable[5],(int)globalState.variable[6],(int)globalState.variable[7],(int)globalState.variable[8],(int)globalState.variable[9],
               (int)globalState.variable[10],(int)globalState.variable[11],(int)globalState.variable[12],(int)globalState.variable[13],(int)globalState.variable[14],
               (int)globalState.variable[15],(int)globalState.variable[16],(int)globalState.variable[17],(int)globalState.variable[18],(int)globalState.variable[19]);
         return telnetBuffer;
      }
      return telnetBuffer;
   }
   else if (strncmp(cmd,"cax",3)==0)
   {
      return handleAxisCommand(cmd+3,pcb);
   }
   logpf(false,"unknown command");
   return TELNET_ERROR"unknown command\r\n";
}


char *handleASCIICommand(char *cmd,const struct tcp_pcb *pcb)
{
   return doHandleASCIICommand(cmd,pcb);
}


static void network1_checkTelnet(struct tcp_pcb *pcb, struct network1_state *hs)
{
#ifdef ENV_FWTEST
   unsigned char mappedCommand=0;
#endif

   if (telnetState.recvBufferLength>0) // there is something in buffer
   {
      {
         while ((telnetState.recvBufferLength>0) && ((telnetState.recvBufferPtr[0]=='\r') || (telnetState.recvBufferPtr[0]=='\n'))) // remove possibly leading CRLF
         {
            telnetState.recvBufferLength-=1;
            telnetState.recvBufferPtr++;
         }
      }
      telnetState.recvBufferPtr[telnetState.recvBufferLength]=0;
      if ((strstr(telnetState.recvBufferPtr,"\r")) || (strstr(telnetState.recvBufferPtr,"\n")) ||  // there is a full line in buffer which could be a c-command
          (telnetState.recvBufferLength>=NET1_BUFFER_LENGTH)) // ...or the buffer is almost full
      {
         char *pos=strstr(telnetState.recvBufferPtr,"\r\n");  // suchen nach "\r\n"
         for (;;)
         {
            const char *ret=handleASCIICommand(telnetState.recvBufferPtr,pcb);

            if (ret)
            {
               // telnetState.recvBufferLength is set by network1_append_response()
               network1_append_response(pcb,hs,ret);
               if ((evtListCtr>=MAX_EVT_NUM) && (cfgListCtr==255))
                network1_append_response(pcb,hs,TELNET_PROMPT); // show prompt
               if (pos)
               {
                  telnetState.recvBufferPtr=pos+2;
                  pos=strstr(telnetState.recvBufferPtr,"\r\n");
               }
               else break;
            }
            else break;
         }
         telnetState.recvBufferLength=0;
         telnetState.recvBufferPtr=0;
         telnetState.recvBuffer[0]=0;
      }
   }
}
#endif

