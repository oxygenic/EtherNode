#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "string.h"
#include "stdio.h"
#include "main_loop.h"
#include "hardware.h"
#include "debug.h"
#include "stepper.h"

/* MQTT Broker Einstellungen */
#define MQTT_TOPIC_SUB "test/sub"
#define MQTT_TOPIC_PUB "test/pub"

static uint8_t connected=0,subscribed=0;
static mqtt_client_t *client=NULL;

enum mytopic
{
   eNONE=0,
   eDOUT=1,
   eDOUT0_FREQ=2,
   eDOUT0_PULSE=3,
   eDOUT1_FREQ=4,
   eDOUT1_PULSE=5,
   eAXIS0_SPD,
   eAXIS0_ACC,
   eAXIS0_SPOS,
   eAXIS0_MPOS,
   eAXIS0_HPOS,
   eAXIS1_SPD,
   eAXIS1_ACC,
   eAXIS1_SPOS,
   eAXIS1_MPOS,
   eAXIS1_HPOS,
};

static enum mytopic usetopic=eNONE;

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t len)
{
   if      (strstr(topic,"/DOut/0/Freq"))  usetopic=eDOUT0_FREQ;
   else if (strstr(topic,"/DOut/0/Pulse")) usetopic=eDOUT0_PULSE;
   else if (strstr(topic,"/DOut/1/Freq"))  usetopic=eDOUT1_FREQ;
   else if (strstr(topic,"/DOut/1/Pulse")) usetopic=eDOUT1_PULSE;
   else if (strstr(topic,"/DOut")) usetopic=eDOUT;

   else if (strstr(topic,"/Axis/0/Speed")) usetopic=eAXIS0_SPD;
   else if (strstr(topic,"/Axis/0/Acc")) usetopic=eAXIS0_ACC;
   else if (strstr(topic,"/Axis/0/SPos")) usetopic=eAXIS0_SPOS;
   else if (strstr(topic,"/Axis/0/MPos")) usetopic=eAXIS0_MPOS;
   else if (strstr(topic,"/Axis/0/HPos")) usetopic=eAXIS0_HPOS;

   else if (strstr(topic,"/Axis/1/Speed")) usetopic=eAXIS0_SPD;
   else if (strstr(topic,"/Axis/1/Acc")) usetopic=eAXIS0_ACC;
   else if (strstr(topic,"/Axis/1/SPos")) usetopic=eAXIS0_SPOS;
   else if (strstr(topic,"/Axis/1/MPos")) usetopic=eAXIS0_MPOS;
   else if (strstr(topic,"/Axis/1/HPos")) usetopic=eAXIS0_HPOS;
}


static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
   char buf[128];
   static uint32_t freq=0;
          uint32_t axis=1;

   if(len>sizeof(buf)-1) len=sizeof(buf)-1;
   if (len<1) return;
   memcpy(buf, data, len);
   buf[len]=0;
   switch (usetopic)
   {
      case eDOUT:
         globalState.digiOut=(int)strtol(buf+2,NULL,16);
         hw_gpio_set(globalState.digiOut);
         break;
      case eDOUT0_FREQ:
      case eDOUT1_FREQ:
         freq=atoi(buf);
         break;
      case eDOUT0_PULSE:
      {
         const int32_t pulse=atoi(buf);
         hw_pwm_set(0,freq,pulse);
         break;
      }
      case eDOUT1_PULSE:
      {
         const int32_t pulse=atoi(buf);
         hw_pwm_set(1,freq,pulse);
         break;
      }
      case eAXIS0_SPD:
         axis=0;
      case eAXIS1_SPD:
         const uint32_t spd=atoi(buf);
         if (spd==0) stepper_stop();
         else globalConfig.stepVmax[axis]=spd;
         break;
      case eAXIS0_ACC:
         axis=0;
      case eAXIS1_ACC:
         globalConfig.stepA[axis]=abs(atoi(buf));
         break;
      case eAXIS0_SPOS:
         axis=0;
      case eAXIS1_SPOS:
         stepper_set_position(atoi(buf),axis);
         break;
      case eAXIS0_MPOS:
         axis=0;
      case eAXIS1_MPOS:
         stepper_move(atoi(buf),axis);
         break;
      case eAXIS0_HPOS:
         axis=0;
      case eAXIS1_HPOS:
         stepper_home(atoi(buf),axis);
         break;
      default:
         logpf(0,"illegal topic %d",usetopic);
         break;
   }
   usetopic=eNONE;
}


/* Callback: Subscribe Ack */
static void mqtt_subscribe_cb(void *arg, err_t result)
{
    if(result == ERR_OK) subscribed=1;
    else subscribed=0;
}


static void send_subscribe()
{
   char tmpStr[300+1];

   snprintf(tmpStr,300,"%s/%s/DOut",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/DOut/0/Pulse",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/DOut/0/Freq",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/DOut/1/Pulse",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/DOut/1/Freq",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);

   snprintf(tmpStr,300,"%s/%s/Axis/0/Speed",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/0/Acc",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/0/SPos",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/0/MPos",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/0/HPos",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);

   snprintf(tmpStr,300,"%s/%s/Axis/1/Speed",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/1/Acc",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/1/SPos",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/1/MPos",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);
   snprintf(tmpStr,300,"%s/%s/Axis/1/HPos",globalConfig.mqttTopic,globalState.mqttMachineID);
   mqtt_subscribe(client,tmpStr, 0, mqtt_subscribe_cb, NULL);

   // extend MQTT_REQ_MAX_IN_FLIGHT for more topics!
}

/* Callback: Verbindung zum Broker */
static void mqtt_connection_cb(mqtt_client_t *c, void *arg, mqtt_connection_status_t status)
{
   if(status == MQTT_CONNECT_ACCEPTED)
   {
      connected=1;
      send_subscribe();
   }
   else
   {
      connected=0;
   }
}


static void mqtt_pub_request_cb(void *arg, err_t err)
{

}


void lib_mqtt_publish(const char *topicData,const char *payData)
{
   if ((connected==1) && (subscribed==0)) send_subscribe();
   if ((!client) ||
       (mqtt_publish(client,topicData,(void*)payData,strlen(payData),1,1,mqtt_pub_request_cb,NULL)!=ERR_OK))
   {
      connected=0;
   }
}


uint8_t lib_mqtt_start(const ip_addr_t broker_ip,const uint16_t port,const char *id,const char* user,const char *pass)
{
   if (!connected)
   {
      if (client) mqtt_client_free(client);

      struct mqtt_connect_client_info_t ci;
      memset(&ci, 0, sizeof(ci));
      ci.client_id =id;
      if ((!user) || (user[0]==0)) ci.client_user=NULL;
      else ci.client_user =user;
      if ((!pass) || (pass[0]==0)) ci.client_pass=NULL;
      else ci.client_pass =pass;
      ci.keep_alive=60;

      /* MQTT Client dynamisch anlegen */
      client =(mqtt_client_t*)mqtt_client_new();
      if(client == NULL) return 0;

      const ip_addr_t tip={ntohl(broker_ip.addr)};

       mqtt_client_connect(client, &tip, port, mqtt_connection_cb, NULL, &ci);
       mqtt_set_inpub_callback(client,mqtt_incoming_publish_cb,mqtt_incoming_data_cb,NULL);
       connected=2; // waiting for connect
       subscribed=0; // not yet connected
   }
    return 1;
}
