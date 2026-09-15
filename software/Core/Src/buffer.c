#include <string.h>

#include "main.h"
#include "buffer.h"

struct ufb_data  lFrameBuffer                  ={0,0,(struct uframe*)NULL,LFRAME_BUFFER_SIZE};


char ufb_empty(const struct ufb_data *buffer)
{
   return (buffer->head==buffer->tail);
}


char ufb_push(struct ufb_data *buffer,const struct uframe *data)
{
#ifdef ENV_FWTEST
   if (data->utype==UTYPE_SDELAYS)
   {
      int i=0;
   }
#endif
   if (ufb_free(buffer)<5)
   {
      return 0;
   }
   buffer->buffer[buffer->tail]=*data;
   buffer->tail++;
   if (buffer->tail>=(buffer->size+1)) buffer->tail-=(buffer->size+1);

   return 1;
}


char ufb_push_fast(struct ufb_data *buffer,const struct uframe *data)
{
   if (ufb_free(buffer)<5)
   {
      return 0;
   }
   memcpy(&buffer->buffer[buffer->tail],data,data->length);
   buffer->tail++;
   if (buffer->tail>=(buffer->size+1)) buffer->tail-=(buffer->size+1);

   return 1;
}


struct uframe *ufb_front(const struct ufb_data *buffer)
{
   if (buffer->head==buffer->tail)
   {
      return NULL;
   }
   return &buffer->buffer[buffer->head];
}


struct uframe *ufb_front_next(struct ufb_data *buffer)
{
   uint_fast32_t headNext;

   if (buffer->head==buffer->tail)
    return NULL;

   headNext=buffer->head+1;
   if (headNext>=(buffer->size+1)) headNext-=(buffer->size+1);
   if (headNext==buffer->tail)
    return NULL;

   return &buffer->buffer[headNext];
}


void ufb_pop(struct ufb_data *buffer)
{
   if (ufb_empty(buffer)) return;
   buffer->head++;
   if (buffer->head>=(buffer->size+1)) buffer->head-=(buffer->size+1);
}


unsigned int ufb_free(const struct ufb_data *buffer)
{
   int dist;

   if (ufb_empty(buffer)) return buffer->size;
   dist=buffer->head-buffer->tail;
   if (dist<0) dist+=(buffer->size+1);
   return dist-1;
}

