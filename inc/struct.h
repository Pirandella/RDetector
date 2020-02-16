#ifndef _STRUCT_H_
#define _STRUCT_H_

typedef struct{
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    float seconds;
    float ch0;
    float ch1;
    float ch2;
}ecgData;

typedef struct{
    float lowPass;
    float highPass;
    float derivative;
    float square;
    float movingWindowIntegral;
}QRS_Filter;

typedef struct{
   float value;
   int32_t index;
}peakPoint;

typedef struct{
    unsigned int sh;
    unsigned int sm;
    unsigned int ss;
    unsigned int eh;
    unsigned int em;
    unsigned int es;
}Time;

typedef struct{
    unsigned int h;
    unsigned int m;
    float s;
}rrTime;

#endif  //_STRUCT_H_
