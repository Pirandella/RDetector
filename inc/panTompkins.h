#ifndef _QRS_H_
#define _QRS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "struct.h"

#define SAMPLING_RATE       128

int lowPassFilter(int data);
int highPassFilter(int data);
int derivative(int data);
int square(int data);
int movingWindowIntegral(int data);
peakPoint panTompkins(int sample, float value, int bandpass, int square, int integral);
#endif  // _QRS_H_
