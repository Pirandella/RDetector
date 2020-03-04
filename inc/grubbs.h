#ifndef _GRUBBS_H_
#define _GRUBBS_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "struct.h"
#include "conf.h"

#define DATASET_SIZE     32
#define MOVING_AVG       32

int grubbsReject(float *value, rrTime *time, int n, int k);
void grubbsLable(float *value, signed int *label, int n);
int outliner(float *value, rrTime *time, int n);
float movingAvg(float *value);

#endif //_GRUBBS_H_