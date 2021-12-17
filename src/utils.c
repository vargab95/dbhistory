#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "utils.h"

static const char *errorTypeMapping[] = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR"};

FILE *lfptr = NULL;

extern void print_message(print_priority_t priority, const char *format, ...)
{
    va_list args;
    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    if (!lfptr)
    {
        lfptr = fopen(g_dbhistory_configuration.log_file_path, "a");
        if (!lfptr)
        {
            printf("Error during opening log file (%s): %s (%d)\n", g_dbhistory_configuration.log_file_path,
                   strerror(errno), errno);
        }
    }

    if (lfptr && (g_dbhistory_configuration.log_level <= priority))
    {
        time(&timer);
        tm_info = localtime(&timer);

        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(lfptr, "%s - %s - ", buffer, errorTypeMapping[priority]);

        va_start(args, format);
        vfprintf(lfptr, format, args);
        va_end(args);
    }
}
