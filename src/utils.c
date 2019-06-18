#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "utils.h"

static const char * errorTypeMapping[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"
};

extern void print_message(PrintPriority priority, const char * format, ...) {
    va_list args;
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s - %s - ", buffer, errorTypeMapping[priority]);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

