#include <errno.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>

#include "config.h"
#include "db/common.h"
#include "os_dep.h"
#include "sqlite_wrapper.h"
#include "utils.h"
#ifdef DBHISTORY_USE_REGEX
#include "db/read_re.h"
#else
#include "db/read.h"
#endif
#include "db/handler.h"

extern db_return_codes_t db_connect(const char *db_path)
{
    const char create_db_structure_cmd[] = "CREATE TABLE IF NOT EXISTS \
                                            path_map( \
                                                    id INTEGER PRIMARY KEY AUTOINCREMENT,\
                                                    path VARCHAR(%d) NOT NULL UNIQUE \
                                            );\n \
                                            CREATE TABLE IF NOT EXISTS \
                                            history( \
                                                    id INTEGER PRIMARY KEY AUTOINCREMENT, \
                                                    path_id INTEGER NOT NULL REFERENCES path_map(id), \
                                                    command VARCHAR(%d) NOT NULL, \
                                                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP \
                                            );\n \
                                            CREATE TABLE IF NOT EXISTS \
                                            pinnings( \
                                                    id INTEGER PRIMARY KEY AUTOINCREMENT, \
                                                    path_id INTEGER NOT NULL REFERENCES path_map(id), \
                                                    command VARCHAR(%d) NOT NULL \
                                            );";
    char absolute_path[PATH_MAX];
    db_return_codes_t return_code = DB_SUCCESS;

    realpath(db_path, absolute_path);
    print_message(MSG_DEBUG, "Trying to open database: %s\n", absolute_path);
    if (DB_SUCCESS != sql_connect(absolute_path, create_db_structure_cmd))
    {
        return_code = DB_ERROR;
    }

    if (DB_SUCCESS != sql_run_command(NULL, NULL, create_db_structure_cmd))
    {
        return_code = DB_ERROR;
    }

    return return_code;
}

extern db_return_codes_t db_close()
{
    print_message(MSG_TRACE, "Closing sql file\n");
    sql_close();
    return DB_SUCCESS;
}
