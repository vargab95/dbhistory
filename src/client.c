#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "client.h"
#include "db_handler.h"

extern ClientReturnCodes client_add_record(const char * command) {
    char cwd[PATH_MAX];
    ClientReturnCodes return_code = CL_ERROR;
    if (!getcwd(cwd, sizeof(cwd))) {
        return CL_INVALID_PATH;
    }

    if (DB_SUCCESS == db_connect("~/.dbhist.sql")) {
        if (DB_SUCCESS == db_add_record(cwd, command)) {
            return_code = CL_OK;
        }
        db_close();
    }
    return return_code;
}

extern ClientReturnCodes client_get_records(const char * path) {
    directory_history_t dir_hist;
    ClientReturnCodes return_code = CL_ERROR;
    if (DB_SUCCESS == db_connect("~/.dbhist.sql")) {
        if (DB_SUCCESS == db_get_history(path, &dir_hist)) {
            for (unsigned int index = 0u; index < dir_hist.length; index++) {
                history_record_t * record = &(dir_hist.records[index]);
                printf("%d %s %s\n", index, record->path, record->command);
            }
            db_close();
            return_code = CL_OK;
        }
    }
    return return_code;
}
