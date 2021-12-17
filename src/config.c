#include <ctype.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include "config.h"

#define MAX_PARAMETER_NAME_LENGTH 32

dbhistory_configuration_t g_dbhistory_configuration = {.database_path = "/tmp/.dbhistory.sql",
                                                       .log_file_path = "/tmp/.dbhistory.log",
                                                       .deletion_time_threshold = -1,
                                                       .max_command_length = 4096,
                                                       .log_level = MSG_INFO};

static int set_parameter(const char *name, const char *value);
static void go_to_next_line(FILE *fp);

int read_configuration(const char *path)
{
    char tmp;
    char name[MAX_PARAMETER_NAME_LENGTH], value[PATH_MAX];
    FILE *fp;

    fp = fopen(path, "r");

    if (fp == NULL)
    {
        print_message(MSG_ERROR, "Cannot open configuration file %s", path);
        return 0;
    }

    while (!feof(fp))
    {
        int i;

        tmp = getc(fp);
        if (!isalpha(tmp))
        {
            go_to_next_line(fp);
            tmp = getc(fp);
        }

        for (i = 0; i < MAX_PARAMETER_NAME_LENGTH && tmp != '=' && !feof(fp); ++i)
        {
            name[i] = tmp;
            tmp = getc(fp);
        }
        name[i + 1] = '\0';

        if (tmp != '=')
        {
            go_to_next_line(fp);
            continue;
        }

        for (i = 0; i < PATH_MAX && tmp != '\n' && !feof(fp); ++i)
        {
            value[i] = tmp;
            tmp = getc(fp);
        }
        value[i + 1] = '\0';

        if (tmp != '\n' && !feof(fp))
        {
            go_to_next_line(fp);
            continue;
        }

        set_parameter(name, value);
    }

    fclose(fp);

    return 1;
}

static void go_to_next_line(FILE *fp)
{
    while (!(feof(fp) || (getc(fp) == '\n')))
        ;
}

static int set_parameter(const char *name, const char *value)
{
    if (strcmp(name, "database_path") == 0)
    {
        g_dbhistory_configuration.database_path = strdup(value);
    }
    else if (strcmp(name, "log_file_path") == 0)
    {
        g_dbhistory_configuration.log_file_path = strdup(value);
    }
    else if (strcmp(name, "deletion_time_threshold") == 0)
    {
        g_dbhistory_configuration.deletion_time_threshold = atoi(value);
    }
    else if (strcmp(name, "max_command_length") == 0)
    {
        g_dbhistory_configuration.max_command_length = atoi(value);
    }
    else if (strcmp(name, "log_level") == 0)
    {
        int log_level = atoi(value);
        if (log_level < MSG_TRACE || log_level > MSG_ERROR)
        {
            print_message(MSG_ERROR, "Invalid log level was set (%d).", log_level);
        }
        else
        {
            g_dbhistory_configuration.log_level = log_level;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}
