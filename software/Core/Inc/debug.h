#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#ifndef ASSERT
 #define ASSERT(e)
#endif

#define false 0
#define true 1

extern void logInit(void);
extern void logpf(const uint_fast8_t fatal,const char *format,...);
extern const char *logGetErr(void);

#endif //DEBUG_H
