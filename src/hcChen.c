#include "hcChen.h"

bool HC_Chen_detect(float signal){
   ecg_buff[ecg_buff_WR_idx++] = signal;
   sample = ecg_buff_WR_idx;
   ecg_buff_WR_idx %= (M + 1);

   /* High pass filtering */
   if(number_iter < M){
      // first fill buffer with enough points for HP filter
      hp_sum += ecg_buff[ecg_buff_RD_idx];
      hp_buff[hp_buff_WR_idx] = 0;
   }else{
      hp_sum += ecg_buff[ecg_buff_RD_idx];
      int tmp = ecg_buff_RD_idx - M;

      if(tmp < 0) tmp += M + 1;

      hp_sum -= ecg_buff[tmp];

      float y1 = 0;
      float y2 = 0;

      tmp = (ecg_buff_RD_idx - ((M + 1) / 2));
      if(tmp < 0) tmp += M + 1;
      y2 = ecg_buff[tmp];
      y1 = HP_CONSTANT * hp_sum;
      hp_buff[hp_buff_WR_idx] = y2 - y1;
   }

   // done reading ECG buffer, increment position
   ecg_buff_RD_idx++;
   ecg_buff_RD_idx %= (M + 1);

   // done writing to HP buffer, increment position
   hp_buff_WR_idx++;
   hp_buff_WR_idx %= (N + 1);

   /* Low pass filtering */

   // shift in new sample from high pass filter
   lp_sum += hp_buff[hp_buff_RD_idx] * hp_buff[hp_buff_RD_idx];

   if(number_iter < N){
      // first fill buffer with enough points for LP filter
      next_eval_pt = 0;
   }else{
      // shift out oldest data point
      int tmp = hp_buff_RD_idx - N;
      if(tmp < 0) tmp += N + 1;
      lp_sum -= hp_buff[tmp] * hp_buff[tmp];
      next_eval_pt = lp_sum;
   }

   // done reading HP buffer, increment position
   hp_buff_RD_idx++;
   hp_buff_RD_idx %= (N + 1);

   /* Adapative thresholding beat detection */
   // set initial threshold
   if(number_iter < window_size){
       if(next_eval_pt > treshold) treshold = next_eval_pt;
       ++number_iter;
   }

   // check if detection hold off period has passed
   if(triggered){
       trig_time++;
       if((trig_time >= DELAY_TIME) && (next_eval_pt <= treshold)){
           triggered = false;
           trig_time = 0;
       }
       // if(next_eval_pt < treshold){
       //     triggered = false;
       // }
   }

   // find if we have a new max
   if(next_eval_pt > win_max) win_max = next_eval_pt;

   // find if we are above adaptive threshold
   if((next_eval_pt > treshold) && !triggered){
       last_qrs_point = sample;
       triggered = true;
#ifdef LOG
        fprintf(logFile, "\nsample\tnumber_iter\thp_sum\t\tlp_sum\t\tnext_eval_pt\ttreshold\twin_idx\t\ttriggered\ttrig_time\twin_max\t\tR_Peak\tIndex\n");
        fprintf(logFile, "%6d\t%6d\t\t%f\t%6.6f\t%f\t%f\t%6d\t\t%6d\t\t%6d\t\t%f\t", sample, number_iter, hp_sum, lp_sum, next_eval_pt, treshold, win_idx, triggered, trig_time, win_max);
#endif // LOG
       return true;
   }

   // adjust adaptive threshold using max of signal found
   // in previous window
   if(win_idx++ >= window_size){
      // weighting factor for determining the contribution of
      // the current peak value to the threshold adjustment
      float gamma = 0.2;
      //float gamma = (0.2f + 0.15f) / 2.0f; // 0.15 ~ 0.2
      // forgetting factor -
      // rate at which we forget old observations
      float alpha = 0.05;
      // float alpha = 0.01f + ( ((float) rand() / (float) RAND_MAX) * ((0.1f - 0.01f))); // 0~1
      //float alpha = 1.0f*exp(-0.00005f*(sample - last_qrs_point));
      treshold = alpha * gamma * win_max + (1.0f - alpha) * treshold;
      // reset current window ind
      win_idx = 0;
      win_max = -10000000;
   }

#ifdef LOG
    fprintf(logFile, "\nsample\tnumber_iter\thp_sum\t\tlp_sum\t\tnext_eval_pt\ttreshold\twin_idx\t\ttriggered\ttrig_time\twin_max\t\tR_Peak\tIndex\n");
    fprintf(logFile, "%6d\t%6d\t\t%f\t%6.6f\t%f\t%f\t%6d\t\t%6d\t\t%6d\t\t%f\t", sample, number_iter, hp_sum, lp_sum, next_eval_pt, treshold, win_idx, triggered, trig_time, win_max);
#endif // LOG

   return false;
}

void adaptiveDelay(float rr){
    static float rrSum = 0;
    static int rrIdx = 0;

    rrSum += rr;
    if(rrIdx < ADAPTIVE_DELAY_SAMPLES){
        rrIdx++;
    }else{
        rrSum /= ADAPTIVE_DELAY_SAMPLES;
        DELAY_TIME = (rrSum * 0.5) * 100;
        rrIdx = 0;
        rrSum = 0;
    }
}

#ifdef LOG
void logInit(char *dir){
    char *buffer = malloc(sizeof(char) * 100);

    strncat(buffer, dir, (strrchr(dir, '/') - dir));
    strcat(buffer, "_LOG_HcChen.txt");

    logFile = fopen(buffer, "w");
    if(logFile == NULL){
        perror("Log file:");
        exit(4);
    }

    free(buffer);
}

void logDeinit(){
    fclose(logFile);
}
#endif // LOG