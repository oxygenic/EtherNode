#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdbool.h>

#ifndef ENV_FWTEST
 #include "lwipopts.h"
 #include "lwip/debug.h"
 #include "lwip/stats.h"
 #include "lwip/inet.h"
#else
 #ifdef ENV_LINUX
  #include <arpa/inet.h>
 #endif
 #ifdef ENV_WINDOWS
  #define snprintf _snprintf
 #endif //ENV_WINDOWS
 extern void sim_setLP8(const unsigned char lp8);
 #include <assert.h>
#endif //ENV_FWTEST
#include "lwip/tcp.h"

#ifdef ENABLE_TELNET
 #define TELNET_OK     "OK: "
 #define TELNET_ERROR  "ERR: "
 #define TELNET_PROMPT "e> "

 #define TELNET_BUF_MAX (DATA_LENGTH+10)
 extern void network1_init(void);
#endif // ENABLE_TELNET

#ifdef ENABLE_MQTT
 extern void network4_init(void);
#endif

extern bool            isWriteAllowed;

extern void close_connection(void);

extern char *handleASCIICommand(char *cmd,const struct tcp_pcb *pcb);
extern void telnet_append_response(const char *in_response);

#endif

