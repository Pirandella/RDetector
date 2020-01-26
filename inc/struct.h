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
    int lowPass;
    int highPass;
    int derivative;
    int square;
    int movingWindowIntegral;
}QRS_Filter;

typedef struct{
   float value;
   int32_t index;
}peakPoint;

#endif  //_STRUCT_H_
