#include <ctype.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "os_dep.h"
#include "config.h"

#define MAX_PARAMETER_NAME_LENGTH 32

dbhistory_configuration_t g_dbhistory_configuration = {.database_path = NULL,
                                                       .log_file_path = NULL,
                                                       .deletion_time_threshold = -1,
                                                       .max_command_length = 4096,
                                                       .log_level = MSG_INFO};

static dbhistory_configuration_read_result_t set_parameter(const char *name, const char *value);
static void set_default_pathes();
static void go_to_next_line(FILE *fp);

dbhistory_configuration_read_result_t read_configuration(const char *path)
{
    char tmp;
    char name[MAX_PARAMETER_NAME_LENGTH], value[PATH_MAX];
    FILE *fp;

    fp = fopen(path, "r");

    if (fp == NULL)
    {
        set_default_pathes();
        return CNF_FILE_NOT_EXISTS;
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
        name[i] = '\0';

        if (tmp != '=')
        {
            go_to_next_line(fp);
            continue;
        }

        tmp = getc(fp);
        for (i = 0; i < PATH_MAX && tmp != '\n' && !feof(fp); ++i)
        {
            value[i] = tmp;
            tmp = getc(fp);
        }
        value[i] = '\0';

        if (tmp != '\n' && !feof(fp))
        {
            go_to_next_line(fp);
            continue;
        }

        if (set_parameter(name, value) != CNF_OK)
        {
            return CNF_PARSE_ERROR;
        }
    }

    fclose(fp);

    set_default_pathes();

    return CNF_OK;
}

static void go_to_next_line(FILE *fp)
{
    while (!(feof(fp) || (getc(fp) == '\n')))
        ;
}

static dbhistory_configuration_read_result_t set_parameter(const char *name, const char *value)
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
        return CNF_PARSE_ERROR;
    }

    return CNF_OK;
}

static void set_default_pathes()
{
    char buffer[PATH_MAX];
    struct passwd *pw = getpwuid(getuid());

    if (g_dbhistory_configuration.database_path == NULL)
    {
        sprintf(buffer, "%s/.dbhistory.db", pw->pw_dir);
        g_dbhistory_configuration.database_path = strdup(buffer);
    }

    if (g_dbhistory_configuration.log_file_path == NULL)
    {
        sprintf(buffer, "%s/.dbhistory.log", pw->pw_dir);
        g_dbhistory_configuration.log_file_path = strdup(buffer);
    }
}
