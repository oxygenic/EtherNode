
#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "lwip/tcp.h"
#include "string.h"
#include "main_loop.h"
#include "hardware.h"
#include "stepper.h"

#include <string.h>

static const char TEXT_PLAIN_HDR[] =
   "HTTP/1.1 200 OK\r\n"
   "Content-Type: text/plain; charset=utf-8\r\n"
   "Cache-Control: no-cache, no-store, must-revalidate\r\n"
   "Pragma: no-cache\r\n"
   "Expires: 0\r\n"
   "Content-length: %d\r\n"
   "\r\n";
static const uint32_t TEXT_PLAIN_SIZE=sizeof(TEXT_PLAIN_HDR);

int fs_open_custom(struct fs_file *file, const char *name)
{
   char *httpdBuffer;

   if (strstr(name, "/api/ok.html"))
   {
      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,2);
      strcat(httpdBuffer,"OK");
   }
   else if ((strstr(name, "/api/error.html")) ||
            (strstr(name, "/404.html")))
   {
      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,5);
      strcat(httpdBuffer,"ERROR");
   }
   else if (strcmp(name, "/api/txt/DIn") == 0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,10,"0x%X",globalState.digiINState);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strcmp(name, "/api/json/DIn") == 0)
   {
      char tmpStr[50+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+50+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,50,"{ \"val\": %u, \"time_ms\": %lu }",globalState.digiINState,(long unsigned int)lastDInChange);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+50,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strcmp(name, "/api/txt/EncPos")==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,10,"%u",(unsigned int)lastMotfValue);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strcmp(name, "/api/txt/EncSpd")==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,10,"%d",(int)globalState.motfSpeed);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strcmp(name, "/api/txt/EncAcc")==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,10,"%d",(int)globalState.motfAccel);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strcmp(name, "/api/json/Enc") == 0)
   {
      char tmpStr[70+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+50+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,70,"{ \"pos\": %u, \"spd\": %d, \"acc\": %d, \"time_ms\": %lu }",(unsigned int)globalState.motfValue,(int)globalState.motfSpeed,(int)globalState.motfAccel,(long unsigned int)globalState.encChange);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+50,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/txt/AIn",12)==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      if (name[12]=='0') snprintf(tmpStr,10,"%d",globalState.ain0state);
      else snprintf(tmpStr,10,"%d",globalState.ain1state);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name,"/api/json/AIn",13)==0)
   {
      char tmpStr[50+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+50+1);
      if (!httpdBuffer) return 0;

      if (name[13]=='0') snprintf(tmpStr,50,"{ \"val\": %u, \"time_ms\": %lu }",globalState.ain0state,(long unsigned int)lastAIn0Change);
      else snprintf(tmpStr,50,"{ \"val\": %u, \"time_ms\": %lu }",globalState.ain1state,(long unsigned int)lastAIn1Change);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+50,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/txt/Axis",13)==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      const int32_t axis=name[13]-48;
      if (axis>=MAX_STEP_AXES) return 0;
      snprintf(tmpStr,10,"%d",(int)stepper.current_pos[axis]);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/txt/Spd",12)==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      const int32_t axis=name[12]-48;
      if (axis>=MAX_STEP_AXES) return 0;
      snprintf(tmpStr,10,"%u",(unsigned int)stepper.v);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/json/Axis",14) == 0)
   {
      char tmpStr[50+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+50+1);
      if (!httpdBuffer) return 0;

      const int32_t axis=name[14]-48;
      if (axis>=MAX_STEP_AXES) return 0;
      uint32_t spd;
      if (axis==stepper.currAxis)
      {
         spd=stepper.v;
         if ((spd==0) && (stepper.homing!=HOMESTOP) && (stepper.homing!=HOMEERROR))
          spd=1;
      }
      else spd=0;
      snprintf(tmpStr,50,"{ \"pos\": %d,\"spd\": %u }",(int)stepper.current_pos[axis],(unsigned int)spd);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+50,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/txt/Tim",12)==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      const int32_t num=atoi(name+12);
      if (num>=MAX_TIM_NUM) return 0;
      snprintf(tmpStr,10,"%d",(int)globalState.timer[num]);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/json/Tim",13)==0)
   {
      char tmpStr[250+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+250+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,250,"{ \"Tim0\": %d, \"Tim1\": %d, \"Tim2\": %d, \"Tim3\": %d, \"Tim4\": %d, \"Tim5\": %d, \"Tim6\": %d, \"Tim7\": %d, \"Tim8\": %d, \"Tim9\": %d }",
               (int)globalState.timer[0],(int)globalState.timer[1],(int)globalState.timer[2],(int)globalState.timer[3],(int)globalState.timer[4],
               (int)globalState.timer[5],(int)globalState.timer[6],(int)globalState.timer[7],(int)globalState.timer[8],(int)globalState.timer[9]);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+250,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/txt/Var",12)==0)
   {
      char tmpStr[10+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+10+1);
      if (!httpdBuffer) return 0;

      const int32_t num=atoi(name+12);
      if (num>=MAX_VAR_NUM) return 0;
      snprintf(tmpStr,10,"%d",(int)globalState.variable[num]);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+10,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else if (strncmp(name, "/api/json/Var",13)==0)
   {
      char tmpStr[350+1];

      httpdBuffer=(char*)malloc(TEXT_PLAIN_SIZE+350+1);
      if (!httpdBuffer) return 0;

      snprintf(tmpStr,350,"{ \"Var0\": %d, \"Var1\": %d, \"Var2\": %d, \"Var3\": %d, \"Var4\": %d, \"Var5\": %d, \"Var6\": %d, \"Var7\": %d, \"Var8\": %d, \"Var9\": %d, \"Var10\": %d, \"Var11\": %d, \"Var12\": %d, \"Var13\": %d, \"Var14\": %d, \"Var15\": %d, \"Var16\": %d, \"Var17\": %d, \"Var18\": %d, \"Var19\": %d }",
            (int)globalState.variable[0],(int)globalState.variable[1],(int)globalState.variable[2],(int)globalState.variable[3],(int)globalState.variable[4],
            (int)globalState.variable[5],(int)globalState.variable[6],(int)globalState.variable[7],(int)globalState.variable[8],(int)globalState.variable[9],
            (int)globalState.variable[10],(int)globalState.variable[11],(int)globalState.variable[12],(int)globalState.variable[13],(int)globalState.variable[14],
            (int)globalState.variable[15],(int)globalState.variable[16],(int)globalState.variable[17],(int)globalState.variable[18],(int)globalState.variable[19]);

      snprintf(httpdBuffer,TEXT_PLAIN_SIZE+350,TEXT_PLAIN_HDR,strlen(tmpStr));
      strcat(httpdBuffer,tmpStr);
   }
   else return 0; /* nicht gefunden */

   file->pextension=httpdBuffer;
   file->index = 0;
   file->len   = strlen(httpdBuffer);
   file->flags = 0;
   file->data=NULL;

   return 1;
}

int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
   const int32_t remaining=file->len-file->index;
   if (remaining<=0) return 0;

   const int32_t to_copy = (remaining > count) ? count : remaining;
   memcpy(buffer, ((char*)file->pextension) + file->index, to_copy);
   file->index += to_copy;

   return to_copy;
}

void fs_close_custom(struct fs_file *file)
{
   if (file->pextension) free(file->pextension);
   file->pextension=NULL;
}


/*** POST **********************************************************************************************/

#define MAX_POST_DATA 512   /* je nach Anwendung anpassen */

struct post_context
{
   char buffer[MAX_POST_DATA];
   int  len;
};

/* Wird aufgerufen, wenn POST beginnt */
err_t httpd_post_begin(void *connection,
                       const char *uri,
                       const char *http_request,
                       u16_t http_request_len,
                       int content_len,
                       char *response_uri,
                       u16_t response_uri_len,
                       u8_t *post_auto_wnd)
{
   LWIP_UNUSED_ARG(http_request);
   LWIP_UNUSED_ARG(http_request_len);
   LWIP_UNUSED_ARG(post_auto_wnd);

   const struct tcp_pcb *pcb=http_get_pcb(connection);

   if ((globalConfig.rwip.addr==0) || (pcb->remote_ip.addr==globalConfig.rwip.addr))
   {
      if (strcmp(uri, "/api/txt") == 0)
      {
         struct post_context *ctx = (struct post_context *)mem_malloc(sizeof(struct post_context));
         strncpy(response_uri, "/api/ok.html", response_uri_len);
         if (ctx == NULL) return ERR_MEM;
         ctx->len=0;
         *((struct post_context **)connection) = ctx;
         return ERR_OK;
      }
   }
   strncpy(response_uri, "/api/error.html", response_uri_len);
   return ERR_VAL; /* unbekannte URI → Abbruch */
}

/* Eingehende POST-Daten */
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
   struct post_context *ctx = *((struct post_context **)connection);
   if (ctx == NULL)
   {
      pbuf_free(p);
      return ERR_VAL;
   }

   /* pbuf-Kette in unseren Buffer kopieren */
   struct pbuf *q;
   for (q = p; q != NULL; q = q->next)
   {
      if ((ctx->len + q->len) < MAX_POST_DATA)
      {
         memcpy(&ctx->buffer[ctx->len], q->payload, q->len);
         ctx->len += q->len;
      }
      else
      {
         /* zu viele Daten → ignorieren oder Fehler */
         pbuf_free(p);
         return ERR_MEM;
      }
   }

   pbuf_free(p);
   return ERR_OK;
}


static const char* find_param(const char *body, const char *key)
{
   size_t keylen = strlen(key);
   const char *p = body;

   while (p && *p)
   {
      if (strncmp(p, key, keylen) == 0 && p[keylen] == '=')
      {
         return p + keylen + 1; /* Pointer auf Value */
      }

      /* zum nächsten Parameter springen */
      p = strchr(p, '&');
      if (p)
         p++; /* hinter & weitermachen */
   }
   return NULL;
}


void httpd_post_finished(void *connection,
                         char *response_uri,
                         u16_t response_uri_len)
{
   HAL_StatusTypeDef ret=HAL_ERROR;
   struct post_context *ctx = *((struct post_context **)connection);
   if (ctx == NULL)
      return;

   ctx->buffer[ctx->len] = '\0'; /* terminieren */

   if (strncmp(ctx->buffer,"DOut=0x",7) == 0)
   {
      const char *valstr = ctx->buffer+5;
      globalState.digiOut=(uint8_t)strtoul(valstr,NULL,0);
      hw_gpio_set(globalState.digiOut);
      ret=HAL_OK;
   }
   else if (strncmp(ctx->buffer,"PWM=",4) == 0)
   {
      const char *out_str  =find_param(ctx->buffer, "PWM");
      const char *freq_str =find_param(ctx->buffer, "freq");
      const char *pulse_str=find_param(ctx->buffer, "pulse");
      const int out=atoi(out_str);
      if ((freq_str) && (pulse_str) && (out_str) && (out>=0) && (out<=1))
      {
         const int freq  = atoi(freq_str);
         const int pulse = atoi(pulse_str);

         ret=hw_pwm_set(out,freq,pulse);
      }
   }
   else if (strncmp(ctx->buffer,"Axis=",5) == 0)
   {
      const char *axis_str  =find_param(ctx->buffer, "Axis");
      const char *spos_str =find_param(ctx->buffer, "spos");
      const char *mpos_str =find_param(ctx->buffer, "mpos");
      const char *hpos_str =find_param(ctx->buffer, "hpos");
      const char *spd_str=find_param(ctx->buffer, "spd");
      const char *acc_str=find_param(ctx->buffer, "acc");
      if (axis_str)
      {
         const uint32_t axis=atoi(axis_str);
         if (axis<MAX_STEP_AXES)
         {
            uint32_t spd=0;
            if (spd_str) spd=atoi(spd_str);
            if (acc_str) globalConfig.stepA[axis]=abs(atoi(acc_str));
            if (spos_str) stepper_set_position(atoi(spos_str),axis);
            if (hpos_str)
            {
               if (stepper_home(atoi(hpos_str),axis)) ret=HAL_OK;
            }
            else if ((spd_str) && (spd==0))
            {
               if (stepper_stop()) ret=HAL_OK;
            }
            else if (mpos_str)
            {
               if (spd_str) globalConfig.stepVmax[axis]=spd;
               if (stepper_move(atoi(mpos_str),axis)) ret=HAL_OK;
            }
         }
      }
   }
   else if (strncmp(ctx->buffer,"Tim",3)==0)
   {
      int32_t i;
      char cmpStr[20+1];

      for (i=0; i<MAX_TIM_NUM; i++)
      {
         snprintf(cmpStr,20,"Tim%d",(int)i);
         const char *paramStr=find_param(ctx->buffer,cmpStr);
         if (paramStr) globalState.timer[i]=(uint32_t)atoi(paramStr);
      }
      ret=HAL_OK;
   }
   else if (strncmp(ctx->buffer,"Var",3)==0)
   {
      int32_t i;
      char cmpStr[20+1];

      for (i=0; i<MAX_VAR_NUM; i++)
      {
         snprintf(cmpStr,20,"Var%d",(int)i);
         const char *paramStr=find_param(ctx->buffer,cmpStr);
         if (paramStr) globalState.variable[i]=atoi(paramStr);
      }
      ret=HAL_OK;
   }
   if (ret==HAL_ERROR)
   {
      strncpy(response_uri, "/api/error.html", response_uri_len);
   }
   else
   {
      strncpy(response_uri, "/api/ok.html", response_uri_len);
   }

   /* Kontext freigeben */
   mem_free(ctx);
   *((struct post_context **)connection) = NULL;
}
