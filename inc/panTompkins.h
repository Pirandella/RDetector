#ifndef _QRS_H_
#define _QRS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "struct.h"

#define SAMPLING_RATE   128

float lowPassFilter(float data);
float highPassFilter(float data);
float derivative(float data);
float square(float data);
float movingWindowIntegral(float data);
peakPoint panTompkins(int sample, float value, float bandpass, float square, float integral);
#endif  // _QRS_H_
