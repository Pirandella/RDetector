#ifndef _ARG_PARSER_H_
#define _ARG_PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "struct.h"

#define INPUT_FLAG      0x01
#define ALG_FLAG        0x02
#define EXT_TF_FLAT     0x04
#define CONV_FLAG       0x08
#define PLT_FLAG        0x10
#define FE_FLAG         0x20
#define FS_FLAG         0x40
#define INT_TF_FLAG     0x80

typedef struct{
    char *ecgFile;
    char *qrsFile;
    char *pltFile;
    char *timeFile;
    char *rrFile;
    uint8_t algorithm;
    uint16_t fs;
    uint16_t flag;
    Time *timeCode;
    uint8_t nTime;
}opt;

opt *parseArgs(int argc, char **argv);

#endif  // _ARG_PARSER_H_