#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <sqlite3.h>
#include <linux/limits.h>

#include "sqlite_wrapper.h"
#include "config.h"
#include "utils.h"

static sqlite3 *db;
static unsigned int last_id = 0;

extern int sql_connect(const char *db_path, const char *command)
{
    int return_code = 0;
    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE, NULL))
    {
        print_message(MSG_INFO, "Cannot open database file. Trying to create one.\n");
        if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
        {
            print_message(MSG_ERROR, "Cannot create database: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            return_code = sql_run_command(NULL, NULL, (char *)command, PATH_MAX, MAX_COMMAND_LENGTH);
        }
    }

    return return_code;
}

extern int sql_run_command(
    int (*callback)(void *, int, char **, char **),
    void *data,
    const char *command,
    ...)
{
    va_list args;
    int rc;
    char *zErrMsg = 0;
    char buffer[1024] = {0};

    va_start(args, command);
    vsprintf(buffer, command, args);
    va_end(args);

    print_message(MSG_TRACE, "Running SQL command: %s\n", buffer);
    rc = sqlite3_exec(db, buffer, callback, data, &zErrMsg);
    if (SQLITE_OK != rc)
    {
        print_message(MSG_ERROR, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

extern int sql_get_last_insertion_id()
{
    return sqlite3_last_insert_rowid(db);
}

extern void sql_close()
{
    sqlite3_close(db);
}
