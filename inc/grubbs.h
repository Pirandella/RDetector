#ifndef _GRUBBS_H_
#define _GRUBBS_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "struct.h"

#define DATASET_SIZE     32

int grubbs(float *value, rrTime *time, int n, int k);
int outliner(float *value, rrTime *time, int n);

#endif //_GRUBBS_H_