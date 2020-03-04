#include "grubbs.h"

static const float t[33] = {12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228, 2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086, 2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042, 2.040, 2.037};

static float _max(const float *value, int n){
    float max = value[0];
    for(int i = 1; i < n; i++){
        if(value[i] > max) max = value[i];
    }
    return max;
}

static float _min(const float *value, int n){
    float min = value[0];
    for(int i = 1; i < n; i++){
        if(value[i] < min) min = value[i];
    }
    return min;
}

static int _maxIndex(const float value[], int n){
    float max = value[0];
    int index = 0;
    for(int i = 1; i < n; i++){
        if(value[i] > max){
            max = value[i];
            index = i;
        }
    }
    return index;
}

// float *value - Dataset
// int n - Number of points in dataset
// int k - Possible number of outliners in the dataset
int grubbsReject(float *value, rrTime *time, int n, int k){
    float sum = 0.0, mean = 0.0, s = 0.0;
    float max, min, maxMean, minMean;
    float g, gCrit;

    int i, df, maxIndex;
    int p;

    for(i = 0; i < k; i++){
        for(p = 0; p < n; p++) sum += value[p];
        mean = sum / n;
        for(p = 0; p < n; p++) s += (value[p] - mean) * (value[p] - mean);
        s = sqrt(s / (n - 1));

        max = _max(value, n);
        min = _min(value, n);
        maxIndex = _maxIndex(value, n);
        minMean = max - mean;
        maxMean = mean - min;
        g = ((maxMean > minMean) ? maxMean : minMean) / s;
        df = n - 1;
        gCrit = (n - 1) * t[df] / sqrt(n * ((n - 1) + pow(t[df], 2)));

        // Delete oulier element
        if(g > gCrit){
            for(int j = maxIndex; j < n; j++){
                value[j] = value[j + 1];
                time[j] = time[j + 1];
            }
            n--;
        }
        s = 0;
        sum = 0;
    }
    return n;
}

void grubbsLable(float *value, signed int *label, int n){
    float sum = 0.0, mean = 0.0, s = 0.0;
    float g, gCrit;

    int i, df;
    int p;

    for(p = 0; p < n; p++) sum += value[p];
    mean = sum / n;
    for(p = 0; p < n; p++) s += (value[p] - mean) * (value[p] - mean);
    s = sqrt(s / (n - 1));

    df = n - 1;
    gCrit = (n - 1) * t[df] / sqrt(n * ((n - 1) + pow(t[df], 2)));

    for(i = 0; i < n; i++){
        g = ((mean < value[i]) ? (value[i] - mean) : (mean - value[i])) / s;
        if(g > gCrit) label[i] = -1;
        else label[i] = 0;
    }
}

//---------------- Sorting function --------------------------------------------

static int cmp (const void *a, const void *b) {
   return (*(int *)a - *(int *)b);
}

//------------------------------------------------------------------------------

static float quartiles(const float *value, int n, int q){
    int index = n / 4;
    float res = 0;
    switch(q) {
        case 1:
            res = value[index - 1];
            break;
        case 2:
            res = value[index * 2 - 1];
            break;
        case 3:
            res = value[index * 3 - 1];
            break;
        case 4:
            res = value[index * 4 - 1];
            break;
    }
    return res;
}

int outliner(float *value, rrTime *time, int n){
    float data[n];
    float q1, q3, iqr;
    float upperBound, lowerBound;
    int outIndex[n];
    int j = 0;

    // Copy data from one array to another
    for(int i = 0; i < n; i++) data[i] = value[i];
    // Sort data for quartiles function
    qsort(data, n, sizeof(float), cmp);

    q1 = quartiles(value, n, 1);
    q3 = quartiles(value, n, 3);
    iqr = q3 - q1;
    upperBound = q3 + (1.5 * iqr);
    lowerBound = q1 - (1.5 * iqr);
    for(int i = 0; i < n; i++){
        if((value[i] > upperBound) || (value[i] < lowerBound)){
            outIndex[j++] = i;
        }
    }
    // Delete outilers
    for(int i = 0; i < j; i++){
        for(int k = outIndex[i]; k < n; k++) value[k] = value[k + 1];
        --n;
    }

    return n;
}

float movingAvg(float *value){
    static float win[MOVING_AVG];
    float sum = 0.0;
    static int sample = 0;

    if(sample < MOVING_AVG){
        win[sample] = *value;
        sample++;
    }else{
        for(int i = 0; i < MOVING_AVG; i++) sum += win[i];
        for(int i = 0; i < MOVING_AVG; i++) win[i] = win[i + 1];
        win[MOVING_AVG - 1] = *value;
        return (sum / MOVING_AVG);
    }
    return -1000;
}