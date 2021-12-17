#include <stdlib.h>
#include <string.h>

#include "ini.h"

#include "utils.h"

#include "config.h"

dbhistory_configuration_t g_dbhistory_configuration = {.database_path = "/tmp/.dbhistory.sql",
                                                       .log_file_path = "/tmp/.dbhistory.log",
                                                       .deletion_time_threshold = -1,
                                                       .max_command_length = 4096,
                                                       .log_level = MSG_INFO};

static int handler(void *user, const char *section, const char *name, const char *value);

int read_configuration(const char *path)
{
    if (ini_parse(path, handler, &g_dbhistory_configuration) < 0)
    {
        print_message(MSG_ERROR, "Cannot open configuration file %s", path);
        return 0;
    }

    return 1;
}

static int handler(void *user, const char *section, const char *name, const char *value)
{
    dbhistory_configuration_t *config = (dbhistory_configuration_t *)user;

    if (strcmp(name, "database_path") == 0)
    {
        config->database_path = strdup(value);
    }
    else if (strcmp(name, "log_file_path") == 0)
    {
        config->log_file_path = strdup(value);
    }
    else if (strcmp(name, "deletion_time_threshold") == 0)
    {
        config->deletion_time_threshold = atoi(value);
    }
    else if (strcmp(name, "max_command_length") == 0)
    {
        config->max_command_length = atoi(value);
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
            config->log_level = log_level;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}
