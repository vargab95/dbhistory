#include <stdio.h>
#include <stdarg.h>

#include "utils.h"

extern void print_message(PrintPriority priority, const char * format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

