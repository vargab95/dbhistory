#ifndef __SQLITE_WRAPPER_H__
#define __SQLITE_WRAPPER_H__

#include "./common.h"
#include <sqlite3.h>

extern db_return_codes_t sql_connect(const char *db_path, const char *command);
extern db_return_codes_t sql_run_command(int (*callback)(void *, int, char **, char **), void *data,
                                         const char *command, ...);
extern int sql_get_last_insertion_id();
extern void sql_close();

#endif
