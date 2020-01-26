#ifndef _ARG_PARSER_H_
#define _ARG_PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

typedef struct{
    char *ecgFile;
    char *qrsFile;
    uint8_t algorithm;
}opt;

opt *parseArgs(int argc, char **argv);

#endif  // _ARG_PARSER_H_
