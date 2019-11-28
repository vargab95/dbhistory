#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "config.h"
#include "utils.h"
#include "client.h"
#include "db/db_handler.h"

extern ClientReturnCodes client_add_record(const char * command) {
    char cwd[PATH_MAX];
    ClientReturnCodes return_code = CL_ERROR;
    if (!getcwd(cwd, sizeof(cwd))) {
        print_message(MSG_ERROR, "Invalid path detected!\n");
        return CL_INVALID_PATH;
    }

    if (DB_SUCCESS == db_connect(DEFAULT_DATABASE_PATH)) {
        if (DB_SUCCESS == db_add_record(cwd, command)) {
            return_code = CL_OK;
        }
        db_close();
    }
    return return_code;
}

extern ClientReturnCodes client_get_records(const char * path) {
    directory_history_t dir_hist = {0};
    ClientReturnCodes return_code = CL_ERROR;

    // TODO Refactor to be able to use both methods
#ifndef DBHISTORY_USE_REGEX
    char absolute_path[PATH_MAX];
    if (!realpath(path, absolute_path)) {
        print_message(MSG_ERROR, "Invalid path detected! errno: %d\n", errno);
        return CL_INVALID_PATH;
    }
#else
    char * absolute_path = (char *)path;
#endif

    if (DB_SUCCESS == db_connect(DEFAULT_DATABASE_PATH)) {
        if (DB_SUCCESS == db_get_history(absolute_path, &dir_hist)) {
            print_message(MSG_DEBUG, "Directory history length: %d\n", dir_hist.length);
            for (unsigned int index = 0u; index < dir_hist.length; index++) {
                history_record_t * record = &(dir_hist.records[index]);
                printf("%d %s %s\n", record->timestamp, record->path, record->command);
            }
            return_code = CL_OK;
        }
        db_close();
    }
    return return_code;
}
