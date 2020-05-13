#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "utils.h"

static const char *errorTypeMapping[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"};

FILE *lfptr = NULL;

extern void print_message(PrintPriority priority, const char *format, ...)
{
    va_list args;
    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    if (priority < MSG_DEBUG)
        return;

    if (!lfptr)
        lfptr = fopen(g_dbhistory_configuration.log_file_path, "a+");
    if (lfptr)
    {
        time(&timer);
        tm_info = localtime(&timer);

        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(lfptr, "%s - %s - ", buffer, errorTypeMapping[priority]);

        va_start(args, format);
        vfprintf(lfptr, format, args);
        va_end(args);
    }
    else
    {
        printf("Error during opening log file: %s\n", strerror(errno));
    }
}
