#include "argParser.h"

opt *parseArgs(int argc, char **argv){
    if(argc < 2){
        puts("Not enought arguments!");
        exit(1);
    }

    opt *args = malloc(sizeof(opt));
    char opt = 0;

    while((opt = getopt(argc, argv, "s:a:t:f:hcep")) != -1){
        switch(opt){
            case 's':
                args->ecgFile = optarg;
                if(access(args->ecgFile, R_OK | F_OK)){
                    puts("ECG file doen't exist or doesn't have read permissions!");
                    free(args);
                    exit(2);
                }
                args->flag |= INPUT_FLAG;
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
                args->flag |= ALG_FLAG;
                break;
            case 't':
                args->timeFile = optarg;
                if(access(args->timeFile, R_OK | F_OK)){
                    puts("Time code file doen't exist or doesn't have read permissions!");
                    free(args);
                    exit(2);
                }
                args->flag |= EXT_TF_FLAT;
                break;
            case 'c': // If -e argument not specified then create new R-R file then convert it
                args->flag |= CONV_FLAG;
                break;
            case 'p': // Plot already existing file
                args->flag |= PLT_FLAG;
                break;
            case 'e': // If -e argument is set then do not create a new file. Just conver existing one
                args->flag |= FE_FLAG;
                break;
            case 'f': // Sampling freqency
                args->flag |= FS_FLAG;
                break;
            case 'h':
                puts("\t\t\t[R D E T E C T O R]\nMain purpose of this prorgam is to finde R peaks in ECG file and extract usful data.");
                puts("[Input arguments]\n\t-s:\t Address of the ECG file.");
                puts("\t-a:\t QRS algorithm\n\t\t\"pt\" - Pan Tompkins algorithm\n\t\t\"hc\" - Hc Chen algorithm");
                puts("\t-t:\t External file with aFib time codes. If it's not specified then there must be a \"timeCodes.txt\" file in directory with ECG file");
                puts("\t-c:\t Process ECG file and convert output data in hdf5 format to plot it");
                puts("\t-e:\t Convert already existing file. (Used in combintion with -c)");
                puts("\t-p:\t Plot data");
                puts("\t-f:\t Set sampling frequency of ECG (Not in use for now)");
                puts("\t-h:\t Help)");
                puts("[Example]:\n\tRDetector -s [ECG file] -a hc -f 128\n\t\tOutput only result files like QRS & RR no PLT");
                puts("\tRDetector -c -s [ECG file] -a hc -f 128\n\t\tOutput QRS & RR files and convet QRS to hdf5");
                puts("\tRDetector -ce -s [QRS file]\n\t\tConvet already existing QRS file");
                puts("\tRDetector -p -s [PLT file]\n\t\tPlot already exising file");
                puts("\tRDetector -cp -s [ECG file]\n\t\tOutput QRS & RR file convet it to hdf5 and plot it");
                free(args);
                exit(0);
            default:
                free(args);
                exit(1);
        }
    }

//---------------------------------------------------- Flag check --------------------------------------------------

    if(args->flag & PLT_FLAG){ // Plot exising file
        if(!(args->flag & INPUT_FLAG)){
            puts("Input file not specified!");
            free(args);
            exit(1);
        }
        return args;
    }else{
        if((args->flag & CONV_FLAG) && (args->flag & FE_FLAG)){ // Convert existing file
            if(!(args->flag & INPUT_FLAG)){
                puts("Input file not specified!");
                free(args);
                exit(1);
            }

            char *fileName = malloc(sizeof(char) * 100);
            args->pltFile = malloc(sizeof(char) * 255);
            memset(fileName, 0, 100);
            memset(args->pltFile, 0, 255);

            strcpy(fileName, strrchr(args->ecgFile, '/'));
            strncat(args->pltFile, fileName, (strchr(fileName, '_') - fileName));
            strcat(args->pltFile, "_PLT");
            strcpy(fileName, strrchr(args->ecgFile, '_'));
            strncat(args->pltFile, fileName, (strchr(fileName, '.') - fileName));
            strcat(args->pltFile, ".hdf5");
            strcpy(fileName, args->pltFile);
            strncpy(args->pltFile, args->ecgFile, (strrchr(args->ecgFile, '/') - args->ecgFile));
            strcat(args->pltFile, fileName);

            printf("%s\n", args->pltFile);
            free(fileName);
            return args;
        }else if(args->flag & CONV_FLAG){ // Crete R-R file and convert it
            if(!(args->flag & INPUT_FLAG)){ // ECG file not specified
                puts("ECG file not specified!");
                free(args);
                exit(1);
            }else if(!(args->flag & ALG_FLAG)){ // Algorithm not specified
                puts("R detection algorithm not specified!!!");
                free(args);
                exit(1);
            }else if(!(args->flag & EXT_TF_FLAT)){ // Time codes file not specified
                printf("Time codes file not specified!!! Default time code file will be used at ");
                args->flag |= INT_TF_FLAG;
            }else if(!(args->flag & FS_FLAG)){ // Sampling freqency
                puts("Sampling freqency not specified!!!");
                free(args);
                exit(1);
            }
        }else{
            // Create R-R file without convering it
            if(!(args->flag & INPUT_FLAG)){ // ECG file not specified
                puts("ECG file not specified!");
                free(args);
                exit(1);
            }else if(!(args->flag & ALG_FLAG)){ // Algorithm not specified
                puts("R detection algorithm not specified!!!");
                free(args);
                exit(1);
            }else if(!(args->flag & EXT_TF_FLAT)){ // Time codes file not specified
                printf("File with aFib time codes not specified!!! Default time code file will be used at ");
                args->flag |= INT_TF_FLAG;
            }else if(!(args->flag & FS_FLAG)){ // Sampling freqency
                puts("Sampling freqency not specified!!!");
                free(args);
                exit(1);
            }
        }
    }

//---------------------------------------------------------------------------------------------------------------------

    args->qrsFile = malloc(sizeof(char) * 255);
    args->pltFile = malloc(sizeof(char) * 255);
    args->rrFile = malloc(sizeof(char) * 255);
    if(args->flag & INT_TF_FLAG){
        args->timeFile = malloc(sizeof(char) * 255);
        memset(args->timeFile, 0, 255);
    }
    memset(args->qrsFile, 0, 255);
    memset(args->pltFile, 0, 255);
    memset(args->rrFile, 0, 255);

    strncat(args->qrsFile, args->ecgFile, (strrchr(args->ecgFile, '.') - args->ecgFile));
    strncat(args->pltFile, args->ecgFile, (strrchr(args->ecgFile, '.') - args->ecgFile));
    if(args->flag & INT_TF_FLAG){
        strncat(args->timeFile, args->ecgFile, (strrchr(args->ecgFile, '/') - args->ecgFile));
        strcat(args->timeFile, "/timeCodes.txt\0");
        printf("%s\n", args->timeFile);
    }
    strncat(args->rrFile, args->ecgFile, (strrchr(args->ecgFile, '.') - args->ecgFile));

    if(args->algorithm == 0){
        strcat(args->qrsFile, "_QRS_PanTompkins.txt\0");
        strcat(args->pltFile, "_PLT_PanTompkins.hdf5\0");
        strcat(args->rrFile, "_RR_PanTompkins.txt\0");
    }else if(args->algorithm == 1){
        strcat(args->qrsFile, "_QRS_HcChen.txt\0");
        strcat(args->pltFile, "_PLT_HcChen.hdf5\0");
        strcat(args->rrFile, "_RR_HcChen.txt\0");
    }

//------------------------ Parse time codes ------------------------------------
    FILE *timeFile = fopen(args->timeFile, "r");
    if(timeFile == NULL){
        perror("Time code file error:");
        fclose(timeFile);
        free(args->qrsFile);
        free(args->ecgFile);
        free(args);
        exit(2);
    }
    // Count nubmer of time codes
    char *tmp = malloc(sizeof(char) * 32);
    uint8_t i = 0;
    while(!feof(timeFile)){
        fgets(tmp, 32, timeFile);
        i++;
    }
    free(tmp);
    if(i < 1){
        puts("Time code file is empty!!!");
        free(args->qrsFile);
        free(args->pltFile);
        free(args);
        fclose(timeFile);
        exit(2);
    }
    rewind(timeFile);
    // Parse time codes
    args->timeCode = malloc(sizeof(Time) * i);
    i = 0;
    while(!feof(timeFile)){
        // Cheak if line is commented
        if(fgetc(timeFile) == '#'){
            while(fgetc(timeFile) != '\n');
            continue;
        }
        fseek(timeFile, -1, SEEK_CUR);
        fscanf(timeFile, "%d:%d:%d/%d:%d:%d\n", &args->timeCode[i].sh, &args->timeCode[i].sm, &args->timeCode[i].ss, &args->timeCode[i].eh, &args->timeCode[i].em, &args->timeCode[i].es);
        // printf("%d:%d:%d/%d:%d:%d\n", args->timeCode[i].sh, args->timeCode[i].sm, args->timeCode[i].ss, args->timeCode[i].eh, args->timeCode[i].em, args->timeCode[i].es);
        i++;
    }
    args->nTime = i;
    // printf("%d\n", i);
    fclose(timeFile);

    return args;
}