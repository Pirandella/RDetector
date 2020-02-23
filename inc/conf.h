#ifndef _CONF_H_
#define _CONF_H_

#include <stdio.h>
#include <stdlib.h>

// #define LOG // Log of HcChen function work

// If neither of macros defined the the raw RR interval time will be writen to output file
#define GRUBBS // Claculated RR intervals will be filtered using grubbs()
// #define MOVING_AVERAGE // Claculated RR interval will smoothen

#if defined(GRUBBS) && defined(MOVING_AVERAGE)
#error GRUBBS and MOVING_AVERAGE defined at the same time
#endif

#endif // _CONF_H_