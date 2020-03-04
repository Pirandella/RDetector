#ifndef _CONF_H_
#define _CONF_H_

#include <stdio.h>
#include <stdlib.h>

// #define LOG // Log of HcChen function work

// If neither of macros defined the the raw RR interval time will be writen to output file
#define GRUBBS // Claculated RR intervals will be filtered using grubbs()
// #define MOVING_AVERAGE // Claculated RR interval will smoothen

// #define REJECT
#define LABEL

#define ADAPTIVE_DELAY

// Sanity check
#if defined(GRUBBS) && defined(MOVING_AVERAGE)
#error      GRUBBS and MOVING_AVERAGE defined at the same time
#endif

#if defined(REJECT) && defined(LABEL) && defined(GRUBBS)
#error      Select only one option (REJECT or LABLE)
#elif !defined(REJECT) && !defined(LABEL) && defined(GRUBBS)
#error      Neither the REJECT or LABEL variations of grubbs selected
#endif

#endif // _CONF_H_