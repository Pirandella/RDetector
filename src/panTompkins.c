#include "panTompkins.h"

float lowPassFilter(float data){
   static float y1 = 0, y2 = 0, x[26];
   static int n = 12;
   float y0;

   x[n] = x[n + 13] = data;
   y0 = (y1 * 2) - y2 + x[n] - (x[n + 6] * 2) + x[n + 12];
   y2 = y1;
   y1 = y0;
   y0 *= 0.03125;
   if(--n < 0) n = 12;

   return y0;
}

float highPassFilter(float data){
    static float y1 = 0, x[66];
    static int n = 32;
    float y0;

    x[n] = x[n + 33] = data;
    y0 = y1 + x[n] - x[n + 32];
    y1 = y0;
    if(--n < 0) n = 32;

    return (x[n + 16] - (y0 * 0.03125));
}

float derivative(float data){
    static float x_derv[4];
    float y;

    y = (data * 2) + x_derv[3] - x_derv[1] - ( x_derv[0] * 2);
    y *= 0.125;
    for(int i = 0; i < 3; ++i) x_derv[i] = x_derv[i + 1];
    x_derv[3] = data;

    return y;
}

float square(float data){
   return (data * data);
}

float movingWindowIntegral(float data) {
   static const int WINDOW_SIZE = SAMPLING_RATE * 0.05;
   static float x[WINDOW_SIZE];
   static int ptr = 0;
   static float sum = 0;
   float ly;
   float y;

   if(++ptr == WINDOW_SIZE) ptr = 0;
   sum -= x[ptr];
   sum += data;
   x[ptr] = data;
   ly = sum * 0.03125;
   uint32_t MAX_INTEGRAL = 32000; // 4096
   if(ly > MAX_INTEGRAL) y = MAX_INTEGRAL;
   else y = ly;

   return (y);
}

peakPoint panTompkins(int sample, float value, float bandpass, float square, float integral){
    static const int QRS_TIME = SAMPLING_RATE * 0.1;
    static const int SEARCH_BACK_TIME = SAMPLING_RATE * 1.66f;

    static float bandpass_buffer[SEARCH_BACK_TIME],integral_buffer[SEARCH_BACK_TIME];
    static peakPoint peak_buffer[SEARCH_BACK_TIME];
    static float square_buffer[QRS_TIME];
    static long unsigned last_qrs = 0, last_slope = 0, current_slope = 0;
    static float peak_i = 0, peak_f = 0, threshold_i1 = 0, threshold_i2 = 0, threshold_f1 = 0, threshold_f2 = 0, spk_i = 0, spk_f = 0, npk_i = 0, npk_f = 0;
    static bool qrs, regular = true, prev_regular;
    static float rr1[8] = {0}, rr2[8] = {0}, rravg1, rravg2, rrlow = 0, rrhigh = 0, rrmiss = 0;

    peakPoint result;
    result.index = 0;

    peak_buffer[sample % SEARCH_BACK_TIME].index = sample;
    peak_buffer[sample % SEARCH_BACK_TIME].value = value;
    bandpass_buffer[sample % SEARCH_BACK_TIME] = bandpass;
    integral_buffer[sample % SEARCH_BACK_TIME] = integral;
    square_buffer[sample % QRS_TIME] = square;

    // If the current signal is above one of the thresholds (integral or filtered signal), it's a peak candidate.
    if(integral >= threshold_i1 || bandpass >= threshold_f1){
        peak_i = integral;
        peak_f = bandpass;
    }

    // If both the integral and the signal are above their thresholds, they're probably signal peaks.
    if((integral >= threshold_i1) && (bandpass >= threshold_f1)){
        // There's a 200ms latency. If the new peak respects this condition, we can keep testing.
        if(sample > last_qrs + SAMPLING_RATE * 0.2f){
            // If it respects the 200ms latency, but it doesn't respect the 360ms latency, we check the slope.
            if(sample <= last_qrs + (long unsigned int)(0.36 * SAMPLING_RATE)){
                // The squared slope is "M" shaped. So we have to check nearby samples to make sure we're really looking
                // at its peak value, rather than a low one.
                float current = sample;
                current_slope = 0;
                for(int j = current - QRS_TIME; j <= current; ++j)
                    if(square_buffer[j % QRS_TIME] > current_slope) current_slope = square_buffer[j % QRS_TIME];

                if(current_slope <= (int)(last_slope / 2)){
                    qrs = false;
                }else{
                    spk_i = 0.125 * peak_i + 0.875 * spk_i;
                    threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
                    threshold_i2 = 0.5 * threshold_i1;

                    spk_f = 0.125 * peak_f + 0.875 * spk_f;
                    threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
                    threshold_f2 = 0.5 * threshold_f1;

                    last_slope = current_slope;
                    qrs = true;

                    result.value = value;
                    result.index = sample;
                }
            }else{
                // If it was above both thresholds and respects both latency periods, it certainly is a R peak.
                float current = sample;
                current_slope = 0;
                for(int j = current - QRS_TIME; j <= current; ++j){
                    if(square_buffer[j % QRS_TIME] > current_slope) current_slope = square_buffer[j % QRS_TIME];
                }

                spk_i = 0.125 * peak_i + 0.875 * spk_i;
                threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
                threshold_i2 = 0.5 * threshold_i1;

                spk_f = 0.125 * peak_f + 0.875 * spk_f;
                threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
                threshold_f2 = 0.5 * threshold_f1;

                last_slope = current_slope;
                qrs = true;

                result.value = value;
                result.index = sample;
            }
        }else{
            // If the new peak doesn't respect the 200ms latency, it's noise. Update thresholds and move on to the next sample.
            peak_i = integral;
            npk_i = 0.125 * peak_i + 0.875 * npk_i;
            threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
            threshold_i2 = 0.5 * threshold_i1;
            peak_f = bandpass;
            npk_f = 0.125 * peak_f + 0.875 * npk_f;
            threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
            threshold_f2 = 0.5 * threshold_f1;
            qrs = false;

            return result;
        }
    }

    // If a QRS complex was detected, the RR-averages must be updated.
    if(qrs){
        // Add the newest RR-interval to the buffer and get the new average.
        rravg1 = 0;
        for(int i = 0; i < 7; ++i){
            rr1[i] = rr1[i + 1];
            rravg1 += rr1[i];
        }
        rr1[7] = sample - last_qrs;
        last_qrs = sample;
        rravg1 += rr1[7];
        rravg1 *= 0.125;

        // If the newly-discovered RR-average is normal, add it to the "normal" buffer and get the new "normal" average.
        // Update the "normal" beat parameters.
        if((rr1[7] >= rrlow) && (rr1[7] <= rrhigh)){
            rravg2 = 0;
            for(int i = 0; i < 7; ++i){
                rr2[i] = rr2[i + 1];
                rravg2 += rr2[i];
            }
            rr2[7] = rr1[7];
            rravg2 += rr2[7];
            rravg2 *= 0.125;
            rrlow = 0.92 * rravg2;
            rrhigh = 1.16 * rravg2;
            rrmiss = 1.66 * rravg2;
        }

        prev_regular = regular;
        if(rravg1 == rravg2){
            regular = true;
        }else{
            // If the beat had been normal but turned odd, change the thresholds.
            regular = false;
            if(prev_regular){
                threshold_i1 /= 2;
                threshold_f1 /= 2;
            }
        }
    }else{
        // If no R-peak was detected, it's important to check how long it's been since the last detection.
        float current = sample;
        // If no R-peak was detected for too long, use the lighter thresholds and do a back search.
        // However, the back search must respect the 200ms limit and the 360ms one (check the slope).
        if((sample - last_qrs > (long unsigned int)rrmiss) && (sample > last_qrs + SAMPLING_RATE * 0.2f)){
            // If over SEARCH_BACK_TIME of QRS complex
            if((sample - last_qrs) > SEARCH_BACK_TIME) last_qrs = sample;

            int qrs_last_index = 0; // Last point of QRS complex

            for(int i = current - (sample - last_qrs) + SAMPLING_RATE * 0.2f; i < (long unsigned int)current; ++i){
                if((integral_buffer[i % SEARCH_BACK_TIME] > threshold_i2) && (bandpass_buffer[i % SEARCH_BACK_TIME] > threshold_f2)){
                    current_slope = 0;
                    for(int j = current - QRS_TIME; j <= current; ++j){
                        if(square_buffer[j % QRS_TIME] > current_slope){
                            current_slope = square_buffer[j % QRS_TIME];
                        }
                    }

                    if((current_slope < (int)(last_slope / 2)) && (i + sample) < last_qrs + 0.36 * last_qrs){
                        qrs = false;
                    }else if(i - last_qrs > 550){
                        peak_i = integral_buffer[i % SEARCH_BACK_TIME];
                        peak_f = bandpass_buffer[i % SEARCH_BACK_TIME];
                        spk_i = 0.25 * peak_i+ 0.75 * spk_i;
                        spk_f = 0.25 * peak_f + 0.75 * spk_f;
                        threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
                        threshold_i2 = 0.5 * threshold_i1;
                        last_slope = current_slope;
                        threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
                        threshold_f2 = 0.5 * threshold_f1;
                        // If a signal peak was detected on the back search, the RR attributes must be updated.
                        // This is the same thing done when a peak is detected on the first try.
                        // RR Average 1
                        rravg1 = 0;
                        for(int j = 0; j < 7; ++j){
                            rr1[j] = rr1[j + 1];
                            rravg1 += rr1[j];
                        }
                        rr1[7] = sample - (current - i) - last_qrs;
                        qrs = true;
                        qrs_last_index = i;
                        last_qrs = sample - (current - i);
                        rravg1 += rr1[7];
                        rravg1 *= 0.125;

                        //RR Average 2
                        if((rr1[7] >= rrlow) && (rr1[7] <= rrhigh)){
                            rravg2 = 0;
                            for (int i = 0; i < 7; ++i){
                                rr2[i] = rr2[i + 1];
                                rravg2 += rr2[i];
                            }
                            rr2[7] = rr1[7];
                            rravg2 += rr2[7];
                            rravg2 *= 0.125;
                            rrlow = 0.92 * rravg2;
                            rrhigh = 1.16 * rravg2;
                            rrmiss = 1.66 * rravg2;
                        }

                        prev_regular = regular;
                        if(rravg1 == rravg2){
                            regular = true;
                        }else{
                            regular = false;
                            if(prev_regular){
                                threshold_i1 /= 2;
                                threshold_f1 /= 2;
                            }
                        }
                        break;
                    }
                }
            }

            if(qrs) return peak_buffer[qrs_last_index % SEARCH_BACK_TIME];
        }

        // Definitely no signal peak was detected.
        if(!qrs){
            // If some kind of peak had been detected, then it's certainly a noise peak. Thresholds must be updated accordinly.
            if((integral >= threshold_i1) || (bandpass >= threshold_f1)){
                peak_i = integral;
                npk_i = 0.125 * peak_i + 0.875 * npk_i;
                threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
                threshold_i2 = 0.5 * threshold_i1;
                peak_f = bandpass;
                npk_f = 0.125 * peak_f + 0.875 * npk_f;
                threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
                threshold_f2 = 0.5 * threshold_f1;
            }
        }
    }

    return result;
}
