#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>
#include <linux/limits.h>

#define __USE_XOPEN
#include <time.h>

#include "utils.h"
#include "config.h"
#include "sqlite_wrapper.h"
#include "db_handler.h"


static int get_path_id_callback(void * data, int argc, char ** argv, char ** col_names);
static uint32_t get_path_id(const char * path);
static int get_record_count_callback(void * data, int argc, char ** argv, char ** col_names);
static int get_records_callback(void * data, int argc, char ** argv, char ** col_names);

extern DBReturnCodes db_connect(const char * db_path) {
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
    DBReturnCodes return_code = DB_SUCCESS;

    realpath(db_path, absolute_path);
    print_message(MSG_DEBUG, "Trying to open database: %s\n", absolute_path);
    if (SQLITE_OK != sql_connect(absolute_path, create_db_structure_cmd)) {
        return_code = DB_ERROR;
    }

    return return_code;
}

extern DBReturnCodes db_add_record(const char * path, const char * command) {
    const char insert_path_cmd[] = "INSERT INTO path_map(path) VALUES (\"%s\");";
    const char insert_history_record_cmd[] = "INSERT INTO history(path_id, command) VALUES(%d, \"%s\");";
    uint32_t path_id;
    if (SQLITE_OK == sql_run_command(NULL, NULL, insert_path_cmd, path)) {
        print_message(MSG_DEBUG, "Using last row insert id.\n");
        path_id = sql_get_last_insertion_id();
    } else {
        path_id = get_path_id(path);
        if (0 == path_id) {
            print_message(MSG_ERROR, "Systematic software failure. Unknown path id after insert operation.\n");
            return DB_ERROR;
        }
    }
    return sql_run_command(NULL, NULL, insert_history_record_cmd, path_id, command);
}

static int get_record_count_callback(void * data, int argc, char ** argv, char ** col_names) {
    int * path_id_ptr = (int*)data;
    *path_id_ptr = atoi(argv[0]);
    print_message(MSG_DEBUG, "Record count: %d\n", *path_id_ptr);
    return 0;
}

static int get_records_callback(void * data, int argc, char ** argv, char ** col_names) {
    directory_history_t * history = data;
    size_t str_size;
    struct tm tm;

    str_size = strlen(argv[0]) + 1;
    history->records[history->length].path = (char*)malloc(str_size * sizeof(char));
    memcpy((char*const)history->records[history->length].path, argv[0], str_size);

    str_size = strlen(argv[1]) + 1;
    history->records[history->length].command = (char*)malloc(str_size * sizeof(char));
    memcpy((char*const)history->records[history->length].command, argv[1], str_size);

    strptime(argv[2], "%Y-%m-%d %H:%M:%S", &tm);
    history->records[history->length].timestamp = mktime(&tm);
    history->length++;
	return 0;
}

extern DBReturnCodes db_get_history(const char * path, directory_history_t * history) {
    uint32_t path_id = get_path_id(path);
    if (0 == path_id) {
        print_message(MSG_INFO, "There is no history record for this folder.\n");
        return DB_EMPTY;
    } else {
        const char get_records_cmd[] = "SELECT path, command, timestamp FROM history INNER JOIN path_map ON path_map.id = history.path_id WHERE path_id = %d;";
        const char count_records_cmd[] = "SELECT count(*) FROM history WHERE path_id = %d;";
        int record_cnt = 0;
        if (SQLITE_OK != sql_run_command(get_record_count_callback, &record_cnt, count_records_cmd, path_id)) {
            return DB_ERROR;
        }

        if (0 == record_cnt) {
            return DB_EMPTY;
        }

        history->records = (history_record_t *)malloc(sizeof(history_record_t) * record_cnt);
        history->length = 0;

        if (SQLITE_OK != sql_run_command(get_records_callback, history, get_records_cmd, path_id)) {
            return DB_ERROR;
        }
		return DB_SUCCESS;
    }
}

extern DBReturnCodes db_close() {
    print_message(MSG_TRACE, "Closing sql file\n");
    sql_close();
    return DB_SUCCESS;
}

static int get_path_id_callback(void * data, int argc, char ** argv, char ** col_names) {
    int * path_id_ptr = (int*)data;
    if (!data) {
        print_message(MSG_ERROR, "Null ptr was received in %s\n", __FUNCTION__);
        return 1;
    }
    *path_id_ptr = atoi(argv[0]);
    print_message(MSG_DEBUG, "Path id: %d\n", *path_id_ptr);
    return 0;
}

static uint32_t get_path_id(const char * path) {
    const char get_path_id_cmd[] = "SELECT * FROM path_map WHERE path = \"%s\" LIMIT 1;";
    char buffer[sizeof(get_path_id_cmd) + PATH_MAX] = {0};
    int path_id = 0;
    if(sql_run_command(get_path_id_callback, &path_id, get_path_id_cmd, path) != SQLITE_OK) {
        print_message(MSG_ERROR, "Cannot find id for path %s\n", path);
    }
    return path_id;
}
