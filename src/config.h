#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <linux/limits.h>

typedef struct
{
    char *database_path;
    char *log_file_path;
    unsigned int daemon_tick_time;
    size_t max_command_length;
} dbhistory_configuration_t;

extern dbhistory_configuration_t g_dbhistory_configuration;

extern int read_configuration(const char *path);

#endif