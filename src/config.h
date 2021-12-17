#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <linux/limits.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

typedef struct
{
    char *database_path;
    char *log_file_path;
    size_t max_command_length;
    print_priority_t log_level;

    // in seconds. -1 means, don't delete
    time_t deletion_time_threshold;
} dbhistory_configuration_t;

extern dbhistory_configuration_t g_dbhistory_configuration;

extern int read_configuration(const char *path);

#endif
