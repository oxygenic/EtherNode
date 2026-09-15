#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef ENV_FWTEST
 #define USE_UART_STDIO
#endif

#ifndef USE_UART_STDIO
// #define USE_UART_STDIO
#endif

#include "debug.h"
#include "buffer.h"
#include "main.h"

#ifdef ENV_WINDOWS
 #define snprintf _snprintf
#endif

//#define WRITE_LOG

static char eText[DATA_LENGTH+1];

void logInit(void)
{
   lFrameBuffer.buffer=(struct uframe*)malloc((LFRAME_BUFFER_SIZE+1)*sizeof(struct uframe));
   if (!lFrameBuffer.buffer) logpf(true,"No memory for log buffer");
   eText[0]=0;
}


const char *logGetErr(void)
{
    return eText;
}

void logpf(const uint_fast8_t ferr,const char *format,...)
{
   va_list  arglist;
   char     sText[DATA_LENGTH+1];

#ifndef ENV_FWTEST
#ifndef USE_UART_STDIO
   if (!lFrameBuffer.buffer) return;
#endif
#endif
   va_start(arglist,format);
   vsnprintf(sText,DATA_LENGTH,format,arglist);
   va_end(arglist);
#ifdef ENV_FWTEST
   sText[DATA_LENGTH]=0;
#endif
   if (ferr)
   {
/*#ifdef WRITE_LOG
	  FIL fileObject;

      f_open(&fileObject,"0:/debug.log",FA_CREATE_ALWAYS);
      f_close(&fileObject);
#endif*/
      if (eText[0]==0)
       strncpy(eText,sText,DATA_LENGTH);
   }
   if (lFrameBuffer.buffer)
   {
      if (ufb_free(&lFrameBuffer)<=1)
      {
      }
      else
      {
         struct uframe logFrame;
         logFrame.length=offsetof(struct uframe,d)+sizeof(logFrame.d.rawblock);
         if (ferr)
         {
            const size_t max = DATA_LENGTH - sizeof("ERROR: ");

            snprintf((char*)logFrame.d.rawblock,DATA_LENGTH,"ERROR: %.*s",(int)max,(char*)sText);
//          snprintf((char*)logFrame.d.rawblock,DATA_LENGTH,"ERROR: %s",(char*)sText);
         }
         else strncpy((char*)logFrame.d.rawblock,(char*)sText,DATA_LENGTH);
         ufb_push(&lFrameBuffer,&logFrame);
      }
   }
}


