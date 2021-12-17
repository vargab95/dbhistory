#include <linux/limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "cleaner.h"
#include "client.h"
#include "config.h"
#include "utils.h"

typedef enum return_codes_t
{
    EC_MISSING_CONFIGURATION = -4,
    EC_INVALID_ARG = -3,
    EC_MISSING_ARG = -2,
    EC_SYS_SW_ERR = -1,
    EC_OK = 0
} return_codes_t;

typedef enum command_type_t
{
    DBHISTORY_NOT_SELECTED = 0,
    DBHISTORY_LIST,
    DBHISTORY_ADD,
    DBHISTORY_SEARCH,
    DBHISTORY_CLEANER,
    DBHISTORY_HELP
} command_type_t;

typedef struct
{
    char configuration_file_path[PATH_MAX];

    command_type_t type;
    union
    {
        struct
        {
            const char *command;
        } add;
        struct
        {
            const char *path;
        } list;
        struct
        {
            const char *pattern;
        } search;
        struct
        {
            const char *program_name;
        } help;
    } argument;
} dbhistory_command_t;

static void print_help(const char *name);
static void get_default_config_file_path(char *configuration_file_path);
static return_codes_t process_arguments(const int argc, const char **argv, dbhistory_command_t *operation_type);
static return_codes_t execute_command(const dbhistory_command_t *command);

int main(int argc, const char **argv)
{
    return_codes_t return_code = EC_SYS_SW_ERR;
    dbhistory_command_t command = {.type = DBHISTORY_NOT_SELECTED};

    get_default_config_file_path(command.configuration_file_path);

    if ((return_code = process_arguments(argc, argv, &command)) != EC_OK)
    {
        print_message(MSG_ERROR, "Operation failed");
        return return_code;
    }

    if (!read_configuration(command.configuration_file_path))
    {
        puts("DBHistory error - Could not read configuration file.");
        printf("Check %s for more details\n", g_dbhistory_configuration.log_file_path);
        print_message(MSG_ERROR, "Cannot read configuration file: %s\n", command.configuration_file_path);
        return EC_MISSING_CONFIGURATION;
    }

    return_code = execute_command(&command);

    return return_code;
}

static void get_default_config_file_path(char *configuration_file_path)
{
    struct passwd *pw = getpwuid(getuid());
    sprintf(configuration_file_path, "%s/%s", pw->pw_dir, ".dbhistory.ini");
}

static return_codes_t process_arguments(const int argc, const char **argv, dbhistory_command_t *command)
{
    int opt;

    while (opt = getopt(argc, (char **)argv, "a:s:pc:h"))
    {
        switch (opt)
        {
        case -1:
            command->type = DBHISTORY_LIST;
            command->argument.list.path = (argc > 1) ? argv[argc - 1] : ".";
            return EC_OK;

        case 'c':
            strcpy(command->configuration_file_path, optarg);
            break;

        case 's':
            command->type = DBHISTORY_SEARCH;
            command->argument.search.pattern = strdup(optarg);
            return EC_OK;

        case 'h':
            command->type = DBHISTORY_HELP;
            command->argument.help.program_name = argv[0];
            return EC_OK;

        case 'p':
            command->type = DBHISTORY_CLEANER;
            return EC_OK;

        case 'a':
            command->type = DBHISTORY_ADD;
            command->argument.add.command = strdup(optarg);
            return EC_OK;

        case ':':
            print_message(MSG_INFO, "Missing value.\n");
            return EC_MISSING_ARG;

        case '?':
            print_message(MSG_INFO, "Unknown option: %c\n", optopt);
            return EC_INVALID_ARG;
        }
    }

    return EC_SYS_SW_ERR;
}

static void print_help(const char *name)
{
    printf("Directory based command history.\n\n"
           "Usage: %s [OPTIONS] [COMMAND]\n"
           "    -h Shows this help message\n"
           "    -c Specify configuration file\n"
           "    -p Cleans up the database\n"
           "    -a Adds the COMMAND to the history db\n"
           "    -s Search by applying given regex to pathes\n",
           name);
}

static return_codes_t execute_command(const dbhistory_command_t *command)
{
    switch (command->type)
    {
    case DBHISTORY_LIST:
        print_message(MSG_INFO, "List history of %s directory.\n",
                      (strcmp(".", command->argument.list.path) == 0) ? "current" : command->argument.list.path);
        client_get_records(command->argument.list.path);
        break;
    case DBHISTORY_ADD:
        print_message(MSG_INFO, "Adding entry %s\n", command->argument.add.command);
        client_add_record(command->argument.add.command);
        break;
    case DBHISTORY_SEARCH:
        print_message(MSG_INFO, "List history for %s regexp.\n", command->argument.search.pattern);
        client_search_records(command->argument.search.pattern);
        break;
    case DBHISTORY_CLEANER:
        print_message(MSG_INFO, "Starting cleaner\n");
        cleaner_run();
        break;
    case DBHISTORY_HELP:
        print_help(command->argument.help.program_name);
        break;
    default:
        print_message(MSG_ERROR, "%d is an invalid command!", command->type);
        return EC_SYS_SW_ERR;
    }

    return EC_OK; // TODO replace with specific return code
}
