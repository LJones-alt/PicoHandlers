#ifndef DMPRETTYPRINT_H
#define DMPRETTYPRINT_H

#include "AnsiColours.h"
#include "stdio.h"

#define PRINT_ERR(originclass, fmt, ...)  \
    printf(originclass "\t" RED "[ERROR]" CRESET REDB "\t" fmt CRESET "\n", ##__VA_ARGS__)

#define PRINT_INFO(originclass, fmt, ...)  \
    printf(originclass "\t[INFO]\t" fmt "\n", ##__VA_ARGS__)

#define PRINT_VERBOSE(originclass, fmt, ...)  \
    printf(originclass "\t[DEBUG]\t" fmt "\n", ##__VA_ARGS__)

#define PRINT_WARN(originclass, fmt, ...)  \
    printf(originclass "\t" YEL "[WARN]" CRESET "\t" fmt "\n", ##__VA_ARGS__)

#define PRINT_SUCCESS(originclass, fmt, ...)  \
    printf(originclass "\t" GRN "[SUCCS]" CRESET "\t" fmt "\n", ##__VA_ARGS__)

#define PRINT_DEBUG_HIGHLIGHT(originclass, fmt, ...)  \
    printf(originclass "\t" "[DEBUG]" BLUB "\t" fmt CRESET "\n", ##__VA_ARGS__)

#define PRINT_HEADING(headingName)  \
    printf("====================[ " headingName " ]====================\n")



#endif