#include <errno.h>
#include <linux/limits.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>

#include "config.h"
#include "db/common.h"
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
                                            );";
    char absolute_path[PATH_MAX];
    db_return_codes_t return_code = DB_SUCCESS;

    realpath(db_path, absolute_path);
    print_message(MSG_DEBUG, "Trying to open database: %s\n", absolute_path);
    if (SQLITE_OK != sql_connect(absolute_path, create_db_structure_cmd))
    {
        return_code = DB_ERROR;
    }

    return return_code;
}

extern db_return_codes_t db_add_record(const char *path, const char *command)
{
    const char insert_path_cmd[] = "INSERT INTO path_map(path) VALUES (\"%s\");";
    const char insert_history_record_cmd[] = "INSERT INTO history(path_id, command) VALUES(%d, \"%s\");";
    uint32_t path_id;
    if (SQLITE_OK == sql_run_command(NULL, NULL, insert_path_cmd, path))
    {
        print_message(MSG_DEBUG, "Using last row insert id.\n");
        path_id = sql_get_last_insertion_id();
    }
    else
    {
        path_id = get_path_id(path);
        if (0 == path_id)
        {
            print_message(MSG_ERROR, "Systematic software failure. Unknown path id after insert operation.\n");
            return DB_ERROR;
        }
    }
    return sql_run_command(NULL, NULL, insert_history_record_cmd, path_id, command);
}

extern db_return_codes_t db_close()
{
    print_message(MSG_TRACE, "Closing sql file\n");
    sql_close();
    return DB_SUCCESS;
}
