#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "daemon.h"
#include "client.h"
#include "utils.h"

typedef enum {
    EC_INVALID_ARG,
    EC_MISSING_ARG,
    EC_SYS_SW_ERR,
    EC_OK = 0,
    EC_HELP
} ReturnCodes;

static void print_help(const char * name);
static ReturnCodes process_arguments(const int argc, const char **argv);

int main(int argc, const char **argv) {
    ReturnCodes return_code = EC_SYS_SW_ERR;
    if ((return_code = process_arguments(argc, argv)) != EC_OK) {
        return return_code;
    }

    return return_code;
}

static ReturnCodes process_arguments(const int argc, const char **argv) {
    int opt = getopt(argc, (char**)argv, ":a:s:u:dh");

    switch(opt)
    {
        /* List directory
            There is no command line argument so
            list the history of the current directory. */
        case -1:
            print_message(0, "List history of current directory\n");
            client_get_records(".");
            return EC_OK;
        /* Help */
        case 'h':
            print_help(argv[0]);
            return EC_HELP;
        /* Daemon
            Starts a daemon instead of opening sqlite
            at every run. */
        case 'd':
            print_message(0, "Starting daemon\n");
            daemon_run();
            return EC_OK;
        /* Add
            Add a new record to the history database. */
        case 'a':
            print_message(0, "Adding entry %s\n", optarg);
            client_add_record(optarg);
            return EC_OK;
        /* Search
            Searches history info based on the specified regex. */
        case 's':
            print_message(0, "Printing messages based on regex %s\n", optarg);
            client_get_records(optarg);
            return EC_OK;
        /* Upper
            Prints history of upper directory and children. */
        case 'u':
            print_message(0, "Printing messages %s folders upper\n", optarg);
            return EC_OK;
        /* Missing value */
        case ':':
            print_message(0, "Missing value.\n");
            return EC_MISSING_ARG;
        /* Unknown argument was specified */
        case '?':
            print_message(0, "Unknown option: %c\n", optopt);
            return EC_INVALID_ARG;
    }
    return EC_SYS_SW_ERR;
}

static void print_help(const char * name) {
    print_message(0,
                  "Directory based command history.\n\n"
                  "Usage: %s [OPTIONS] [COMMAND]\n"
                  "\t-h Shows this help message\n"
                  "\t-d Starts a history logger daemon\n"
                  "\t-a Adds the COMMAND to the history db\n"
                  "\t-s Prints logs based on the given regex\n"
                  "\t-u Prints logs n folders upper\n",
                  name);
}
