#ifndef BUFFER_H
#define BUFFER_H

#define BUFFER_TRACE

#include <stdint.h>

#include "main.h"
#include "network.h"
#include "main_loop.h"


#define LFRAME_BUFFER_SIZE         (1000) // log lines buffer


struct
#ifndef ENV_WINDOWS
 __attribute__((__packed__))
#endif
uframe
{
   signed char    identifier;
   unsigned char  length; // correction table is separated so 8 bytes are enough
   union
#ifndef ENV_WINDOWS
   __attribute__((__packed__))
#endif
   {
      unsigned char               rawblock[DATA_LENGTH+1]; // add one byte to have total struct length dividable by 4
   } d;
};

struct ufb_data
{
   uint_fast32_t        head,tail;
   struct uframe       *buffer;
   const uint_fast32_t  size;
};

extern char           ufb_empty(const struct ufb_data *buffer);
extern char           ufb_push(struct ufb_data *buffer,const struct uframe *data);
extern char           ufb_push_fast(struct ufb_data *buffer,const struct uframe *data);
extern char           ufb_push_uframe(const struct uframe *data);
extern char           ufb_push_uframe_fast(const struct uframe *data);
extern struct uframe *ufb_front(const struct ufb_data *buffer);
extern struct uframe *ufb_front_next(struct ufb_data *buffer);
extern void           ufb_pop(struct ufb_data *buffer);
extern unsigned int   ufb_free(const struct ufb_data *buffer);


extern struct ufb_data lFrameBuffer;

#endif //BUFFER_H
