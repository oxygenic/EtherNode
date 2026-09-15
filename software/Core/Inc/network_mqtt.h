#ifndef NETWORK_MQTT_H
#define NETWORK_MQTT_H

extern uint8_t lib_mqtt_start(const ip_addr_t broker_ip,const uint16_t port,const char *id,const char* user,const char *pass);
extern void lib_mqtt_publish(const char *topicData,const char *payData);

#endif
