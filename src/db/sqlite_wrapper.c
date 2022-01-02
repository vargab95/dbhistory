#include <linux/limits.h>
#include <sqlite3.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "../utils.h"
#include "sqlite_wrapper.h"

static sqlite3 *db;
static unsigned int last_id = 0;

extern db_return_codes_t sql_connect(const char *db_path, const char *command)
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
            return_code =
                sql_run_command(NULL, NULL, (char *)command, PATH_MAX, g_dbhistory_configuration.max_command_length);
        }
    }

    return (return_code == SQLITE_OK) ? DB_SUCCESS : DB_ERROR;
}

extern db_return_codes_t sql_run_command(int (*callback)(void *, int, char **, char **), void *data,
                                         const char *command, ...)
{
    va_list args;
    int rc;
    char *zErrMsg = 0;
    char *buffer;

    buffer = malloc(strlen(command) + 2048);

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

    free(buffer);

    return (rc == SQLITE_OK) ? DB_SUCCESS : DB_ERROR;
}

extern int sql_get_last_insertion_id()
{
    return sqlite3_last_insert_rowid(db);
}

extern void sql_close()
{
    sqlite3_close(db);
}
