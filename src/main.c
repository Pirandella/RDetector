#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "argParser.h"
#include "struct.h"
#include "panTompkins.h"
#include "hcChen.h"
#include "grubbs.h"

int main(int argc, char **argv){

    opt *args = malloc(sizeof(opt));
    args = parseArgs(argc, argv);

    ecgData *ecg = malloc(sizeof(ecgData));
    QRS_Filter *qrs = malloc(sizeof(QRS_Filter));
    peakPoint peak;
    int globalIndex = 0;
    int localIndex = 0;
    int peakCount = 0;
    int aFibKnown = 0;
    int aFibFound = 0;
    int aFibFlag = 0;

    int i = 0;
    unsigned int r1 = 0;
    float rr = 0;
    int rrIndex = 0;
    int aFibStartTime = 0;
    int aFibEndTime = 0;

    FILE *ecgFile;
    FILE *qrsFile;
    FILE *rrFile;
    char *tmpBuff = malloc(sizeof(char) * 255);
    memset(tmpBuff, 0, 255);

    if(args->flag & PLT_FLAG){ // Plot existing data
        sprintf(tmpBuff, "python3 ./plot.py %s", args->ecgFile);
        system(tmpBuff);
        free(tmpBuff);
    }else if((args->flag & CONV_FLAG) && (args->flag & FE_FLAG)){ // File already exist
        sprintf(tmpBuff, "python3 ./txtToHdf5.py %s %s", args->ecgFile, args->pltFile);
        system(tmpBuff);
        free(tmpBuff);
    }else{
        ecgFile = fopen(args->ecgFile, "r");
        qrsFile = fopen(args->qrsFile, "w");
        rrFile = fopen(args->rrFile, "w");

        if((ecgFile == NULL) || (qrsFile == NULL) || (rrFile == NULL)){
            perror("File opening error!");
            fclose(ecgFile);
            fclose(qrsFile);
            fclose(rrFile);
            free(args->qrsFile);
            free(args->pltFile);
            free(args->rrFile);
            free(args);
            free(ecg);
            free(qrs);
            free(tmpBuff);
            exit(1);
        }

        fgets(tmpBuff, 100, ecgFile); // Read table title line

        // temporary rrBuffer
        float tmpBuffer[DATASET_SIZE];

        aFibStartTime = (args->timeCode[i].sh * 3600) + (args->timeCode[i].sm * 60) + args->timeCode[i].ss;
        aFibEndTime = (args->timeCode[i].eh * 3600) + (args->timeCode[i].em * 60) + args->timeCode[i].es;
        peak.value = 0.0;
        while(!feof(ecgFile)){
            fscanf(ecgFile, "%d.%d.%d\t%d\t%d\t%f\t%f\t%f\t%f\n", &ecg->year, &ecg->month, &ecg->day, &ecg->hours, &ecg->minutes, &ecg->seconds, &ecg->ch0, &ecg->ch1, &ecg->ch2);
            switch(args->algorithm){
                case 0:
                    qrs->lowPass = lowPassFilter(ecg->ch0);
                    qrs->highPass = highPassFilter(qrs->lowPass);
                    qrs->derivative = derivative(qrs->highPass);
                    qrs->square = square(qrs->derivative);
                    qrs->movingWindowIntegral = movingWindowIntegral(qrs->square);
                    peak = panTompkins(globalIndex, ecg->ch0, qrs->highPass, qrs->square, qrs->movingWindowIntegral);
                    if(peak.value < 0) peak.value = 0.0;
                    break;
                case 1:
                    peak.index = HC_Chen_detect(ecg->ch0);
                    if(peak.index == 1){
                        peak.index = globalIndex;
                        peak.value = ecg->ch0;
                    }else{
                        peak.value = 0.0;
                    }
                    break;
            }

            if(peak.index > 0){
                peak.value = ecg->ch0;
                int fileTimeSec = (ecg->hours * 3600) + (ecg->minutes * 60) + (int)ecg->seconds;
                if(aFibFlag != 2){
                    if((fileTimeSec >= aFibStartTime) && (fileTimeSec <= aFibEndTime)){
                        //printf("\b%c[2K%d:%d:%f\r", 27, ecg->hours, ecg->minutes, ecg->seconds);
                        // fflush(stdout);
                        aFibFlag = 1;
                        aFibKnown = 1;
                    }else if((fileTimeSec > aFibEndTime) && (aFibFlag == 1)){
                        if(i != (args->nTime - 1)){
                            ++i;
                            printf("\x1B[33mNext time interval: %02d:%02d:%02d->%02d:%02d:%02d\x1B[0m\n", args->timeCode[i].sh, args->timeCode[i].sm, args->timeCode[i].ss, args->timeCode[i].eh, args->timeCode[i].em, args->timeCode[i].es);
                            aFibStartTime = (args->timeCode[i].sh * 3600) + (args->timeCode[i].sm * 60) + args->timeCode[i].ss;
                            aFibEndTime = (args->timeCode[i].eh * 3600) + (args->timeCode[i].em * 60) + args->timeCode[i].es;
                            aFibFlag = 0;
                            aFibKnown = 0;
                        }else{
                            printf("\x1B[32mNo more time intervals\x1B[0m\n");
                            aFibFlag = 2;
                            aFibKnown = 0;
                        }
                    }else{
                        aFibKnown = 0;
                    }
                }

                rr = fabsf(rr - localIndex) * 0.0078125;
                r1 = localIndex;
                localIndex = 0;

                if(rrIndex > DATASET_SIZE){
                    int g = grubbs(tmpBuffer, DATASET_SIZE, 24);
                    g = outliner(tmpBuffer, g);
                    for(int k = 0; k < g; k++){
                        if((tmpBuffer[k] > 0.375) && (tmpBuffer[k] < 1.38))
                            fprintf(rrFile, "%02d:%02d:%02f\t%f\t%d\n", ecg->hours, ecg->minutes, ecg->seconds, tmpBuffer[k], aFibKnown);
                    }
                    rrIndex = 0;
                }else{
                    // if((rr > 0.35) && (rr < 1.4))
                        tmpBuffer[rrIndex++] = rr;
                }

                //fprintf(rrFile, "%02d:%02d:%02f\t%f\t%d\n", ecg->hours, ecg->minutes, ecg->seconds, rr, aFibKnown);
            }
            fprintf(qrsFile, "%02d:%02d:%f\t%9d\t%4d\t%f\t%f\t%d\t%d\n", ecg->hours, ecg->minutes, ecg->seconds, globalIndex, peak.index ? ++peakCount : 0, peak.value, ecg->ch0, aFibKnown, aFibFound);
            globalIndex++;
            localIndex++;
        }

        fclose(ecgFile);
        fclose(qrsFile);
        fclose(rrFile);
    }

    if(args->flag & CONV_FLAG){
        sprintf(tmpBuff, "python3 ./txtToHdf5.py %s %s", args->qrsFile, args->pltFile);
        system(tmpBuff);
        free(tmpBuff);
    }else{
        free(tmpBuff);
    }

    free(args->qrsFile);
    free(args->pltFile);
    free(args->rrFile);
    free(args);
    free(ecg);
    free(qrs);
    return 0;
}