#include "argParser.h"

opt *parseArgs(int argc, char **argv){
    if(argc < 2){
        puts("Not enought arguments!");
        exit(1);
    }

    opt *args = malloc(sizeof(opt));
    char opt = 0;
    uint8_t optFlag = 0;

    while((opt = getopt(argc, argv, "s:a:h")) != -1){
        switch(opt){
            case 's':
                args->ecgFile = optarg;
                if(access(args->ecgFile, R_OK | F_OK)){
                    puts("ECG file doen't exist or doesn't have read permissions!");
                    free(args);
                    exit(2);
                }
                optFlag |= (1 << 0);
                break;
            case 'a':
                if(!strcmp(optarg, "pt")){
                    args->algorithm = 0;
                }else if(!strcmp(optarg, "hc")){
                    args->algorithm = 1;
                }else{
                    puts("Wrong -a argument:\n\t\"pt\" - Pan Tompkins algorithm\n\t\"hc\" - Hc Chen algorithm");
                    free(args);
                    exit(1);
                }
                optFlag |= (1 << 1);
                break;
            case 'h':
                puts("-s:\t Address of the ECG file.");
                puts("-a:\t QRS algorithm\n\t\t\"pt\" - Pan Tompkins algorithm\n\t\t\"hc\" - Hc Chen algorithm");
                free(args);
                exit(0);
            default:
                free(args);
                exit(1);
        }
    }

    if(!(optFlag & 0x01)){
        puts("ECG file not specified!");
        free(args);
        exit(1);
    }else if(!(optFlag & 0x02)){
        puts("QRS algorithm not specified!");
        free(args);
        exit(1);
    }

    args->qrsFile = malloc(sizeof(char) * 255);
    memset(args->qrsFile, 0, 255);

    strncat(args->qrsFile, args->ecgFile, (strrchr(args->ecgFile, '.') - args->ecgFile));
    if(args->algorithm == 0) strcat(args->qrsFile, "_QRS_PanTompkins.txt\0");
    else if(args->algorithm == 1) strcat(args->qrsFile, "_QRS_HcChen.txt\0");

    return args;
}
