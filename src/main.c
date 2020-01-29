#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "argParser.h"
#include "struct.h"
#include "panTompkins.h"
#include "hcChen.h"

int main(int argc, char **argv){

    opt *args = malloc(sizeof(opt));
    args = parseArgs(argc, argv);

    ecgData *ecg = malloc(sizeof(ecgData));
    QRS_Filter *qrs = malloc(sizeof(QRS_Filter));
    peakPoint peak;
        int globalIndex = 0;
    int peakCount = 0;
    int aFigKnown = 0;
    int aFibFound = 0;
    FILE *ecgFile;
    FILE *qrsFile;
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

        if((ecgFile == NULL) || (qrsFile == NULL)){
            perror("File error!");
            fclose(ecgFile);
            fclose(qrsFile);
            free(args->qrsFile);
            free(args->pltFile);
            free(args);
            free(ecg);
            free(qrs);
            free(tmpBuff);
            exit(1);
        }

        fgets(tmpBuff, 100, ecgFile);

        // while(!feof(ecgFile)){
        //     fscanf(ecgFile, "%d.%d.%d\t%d\t%d\t%f\t%f\t%f\t%f\n", &ecg->year, &ecg->month, &ecg->day, &ecg->hours, &ecg->minutes, &ecg->seconds, &ecg->ch0, &ecg->ch1, &ecg->ch2);
        //     switch(args->algorithm){
        //         case 0:
        //             qrs->lowPass = lowPassFilter(ecg->ch0);
        //             qrs->highPass = highPassFilter(qrs->lowPass);
        //             qrs->derivative = derivative(qrs->highPass);
        //             qrs->square = square(qrs->derivative);
        //             qrs->movingWindowIntegral = movingWindowIntegral(qrs->square);
        //             peak = panTompkins(globalIndex, ecg->ch0, qrs->highPass, qrs->square, qrs->movingWindowIntegral);
        //             if(peak.value < 0) peak.value = 0.0;
        //             break;
        //         case 1:
        //             // qrs->lowPass = lowPassFilter(ecg->ch0);
        //             // qrs->highPass = highPassFilter(qrs->lowPass);
        //             // peak.index = HC_Chen_detect(qrs->highPass);
        //             peak.index = HC_Chen_detect(ecg->ch0);
        //             if(peak.index == 1){
        //                 peak.index = globalIndex;
        //                 peak.value = ecg->ch0;
        //             }else{
        //                 //peak.index = 0;
        //                 peak.value = 0.0;
        //             }
        //             break;
        //     }
        //
        //     fprintf(qrsFile, "%02d:%02d:%f\t%9d\t%4d\t%f\t%f\t%d\t%d\n", ecg->hours, ecg->minutes, ecg->seconds, globalIndex, peak.value ? (++peakCount) : 0, peak.value, ecg->ch0, aFigKnown, aFibFound);
        //
        //     globalIndex++;
        // }

        fclose(ecgFile);
        fclose(qrsFile);
    }

    // if(args->flag & CONV_FLAG){
    //     sprintf(tmpBuff, "python3 ./txtToHdf5.py %s %s", args->qrsFile, args->pltFile);
    //     system(tmpBuff);
    //     free(tmpBuff);
    // }else{
    //     free(tmpBuff);
    // }

    free(args->qrsFile);
    free(args->pltFile);
    free(args);
    free(ecg);
    free(qrs);
    return 0;
}