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
    int opt = getopt(argc, (char**)argv, ":a:s:dh");

    switch(opt)
    {
        /* List directory
            There is no command line argument so
            list the history of the current directory. */
        case -1:
            print_message(MSG_INFO, "List history of %s directory.\n", (argc > 1) ? argv[1] : "current");
            client_get_records((argc > 1) ? argv[1] : ".");
            return EC_OK;
        case 's':
            print_message(MSG_INFO, "List history for %s regexp.\n", optarg);
            client_search_records(optarg);
            return EC_OK;
        /* Help */
        case 'h':
            print_help(argv[0]);
            return EC_HELP;
        /* Daemon
            Starts a daemon instead of opening sqlite
            at every run. */
        case 'd':
            print_message(MSG_INFO, "Starting daemon\n");
            daemon_run();
            return EC_OK;
        /* Add
            Add a new record to the history database. */
        case 'a':
            print_message(MSG_INFO, "Adding entry %s\n", optarg);
            client_add_record(optarg);
            return EC_OK;
        /* Missing value */
        case ':':
            print_message(MSG_INFO, "Missing value.\n");
            return EC_MISSING_ARG;
        /* Unknown argument was specified */
        case '?':
            print_message(MSG_INFO, "Unknown option: %c\n", optopt);
            return EC_INVALID_ARG;
    }
    return EC_SYS_SW_ERR;
}

static void print_help(const char * name) {
    printf("Directory based command history.\n\n"
           "Usage: %s [OPTIONS] [COMMAND]\n"
           "\t-h Shows this help message\n"
           "\t-d Starts a history maintainer daemon\n"
           "\t-a Adds the COMMAND to the history db\n"
           "\t-s Search by applying given regex to pathes\n",
           name);
}
