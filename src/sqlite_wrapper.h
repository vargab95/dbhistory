#ifndef __SQLITE_WRAPPER_H__
#define __SQLITE_WRAPPER_H__

#include <sqlite3.h>

extern int sql_connect(const char *db_path, const char *command);
extern int sql_run_command(
    int (*callback)(void *, int, char **, char **),
    void *data,
    const char *command,
    ...);
extern int sql_get_last_insertion_id();
extern void sql_close();

#endif
