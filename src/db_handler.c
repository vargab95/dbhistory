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
#include "db_handler.h"

static sqlite3 *db;
static unsigned int last_id = 0;
static const char create_path_map_cmd[] = "CREATE TABLE IF NOT EXISTS \
                                           path_map( \
                                                   id INTEGER PRIMARY KEY AUTOINCREMENT,\
                                                   path VARCHAR(%d) NOT NULL UNIQUE \
                                           );";
static const char create_history_cmd[] = "CREATE TABLE IF NOT EXISTS \
                                          history( \
                                                  id INTEGER PRIMARY KEY AUTOINCREMENT, \
                                                  path_id INTEGER NOT NULL REFERENCES path_map(id), \
                                                  command VARCHAR(%d) NOT NULL, \
                                                  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP \
                                          );";
static const char insert_path_cmd[] = "INSERT INTO path_map(path) VALUES (\"%s\");";
static const char get_max_path_id_cmd[] = "SELECT * FROM path_map ORDER BY id DESC LIMIT 1;";
static const char get_path_id_cmd[] = "SELECT * FROM path_map WHERE path = \"%s\" LIMIT 1;";
static const char insert_history_record_cmd[] = "INSERT INTO history(path_id, command) VALUES(%d, \"%s\");";
static const char get_history_records_cmd[] = "SELECT path, command, timestamp FROM history INNER JOIN path_map ON path_map.id = history.path_id WHERE path_id = %d;";
static const char count_history_records_cmd[] = "SELECT count(*) FROM history WHERE path_id = %d;";

static DBReturnCodes db_initialize();
static DBReturnCodes run_sql_command(const char * command, ...);
static uint32_t get_last_path_map_id();
static uint32_t get_path_id(const char * path);

extern DBReturnCodes db_connect(const char * db_path) {
    char absolute_path[PATH_MAX];
    DBReturnCodes return_code = DB_SUCCESS;

    realpath(db_path, absolute_path);
    print_message(MSG_DEBUG, "Trying to open database: %s\n", absolute_path);
    if (sqlite3_open_v2(absolute_path, &db, SQLITE_OPEN_READWRITE, NULL)) {
        print_message(MSG_INFO, "Cannot open database file. Trying to create one.\n");
        if (sqlite3_open_v2(absolute_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
            print_message(MSG_ERROR, "Cannot create database: %s\n", sqlite3_errmsg(db));
        } else {
            return_code = DB_EMPTY;
        }
    }

    if (DB_EMPTY == return_code) {
        return_code = db_initialize();
    }

    if (DB_SUCCESS == return_code) {
        last_id = get_last_path_map_id();
    }

    return return_code;
}

extern DBReturnCodes db_add_record(const char * path, const char * command) {
    uint32_t path_id;
    if (DB_SUCCESS == run_sql_command(insert_path_cmd, path)) {
        print_message(MSG_DEBUG, "Using last row insert id.\n");
        path_id = sqlite3_last_insert_rowid(db);
    } else {
        path_id = get_path_id(path);
        if (0 == path_id) {
            print_message(MSG_ERROR, "Systematic software failure. Unknown path id after insert operation.\n");
            return DB_ERROR;
        }
    }
    return run_sql_command(insert_history_record_cmd, path_id, command);
}

extern DBReturnCodes db_get_history(const char * path, directory_history_t * history) {
    uint32_t path_id = get_path_id(path);
    if (0 == path_id) {
        print_message(MSG_INFO, "There is no history record for this folder.\n");
        return DB_EMPTY;
    } else {
        sqlite3_stmt *res;
        const char *tail;
        char buffer[256] = {0};
        sprintf(buffer, count_history_records_cmd, path_id);
        if(sqlite3_prepare_v2(db, buffer, -1, &res, &tail) != SQLITE_OK) {
            print_message(MSG_ERROR, "Systematic software error. There is no history record for an existing path: %s\n", path);
            return DB_ERROR;
        }
        sqlite3_step(res);
        history->length = atoi(sqlite3_column_text(res, 0));
        if (0 == history->length) {
            return DB_EMPTY;
        }
        history->records = (history_record_t *)malloc(sizeof(history_record_t) * history->length);

        sprintf(buffer, get_history_records_cmd, path_id);
        if(sqlite3_prepare_v2(db, buffer, -1, &res, &tail) != SQLITE_OK) {
            print_message(MSG_ERROR, "Systematic software error. Error while getting history records for an existing path: %s\n", path);
            return DB_ERROR;
        } else {
            const char * tmp;
            uint32_t idx = 0;
            struct tm tm;
            size_t str_size;
            while(SQLITE_ROW == sqlite3_step(res)) {
                print_message(MSG_DEBUG, "Adding new record\n");
                str_size = strlen(sqlite3_column_text(res, 0));
                history->records[idx].path = (char*)malloc(str_size * sizeof(char));
                memcpy((char*const)history->records[idx].path, sqlite3_column_text(res, 0), str_size);
                str_size = strlen(sqlite3_column_text(res, 1));
                history->records[idx].command = (char*)malloc(str_size * sizeof(char));
                memcpy((char*const)history->records[idx].command, sqlite3_column_text(res, 1), str_size);
                strptime(sqlite3_column_text(res, 2), "%Y-%m-%d %H:%M:%S", &tm);
                history->records[idx].timestamp = mktime(&tm);
                idx++;
            }
        }
        return DB_SUCCESS;
    }
}

extern DBReturnCodes db_close() {
    print_message(MSG_TRACE, "Closing sql file\n");
    sqlite3_close(db);
    return DB_SUCCESS;
}

static DBReturnCodes db_initialize() {
    if (DB_SUCCESS != run_sql_command(create_path_map_cmd, PATH_MAX)) return DB_ERROR;
    if (DB_SUCCESS != run_sql_command(create_history_cmd, MAX_COMMAND_LENGTH)) return DB_ERROR;
    return DB_SUCCESS;
}

static DBReturnCodes run_sql_command(const char * command, ...) {
    va_list args;
    char *zErrMsg = 0;
    char buffer[1024] = {0};

    va_start(args, command);
    vsprintf(buffer, command, args);
    va_end(args);

    print_message(MSG_TRACE, "Running SQL command: %s\n", buffer);
    if (SQLITE_OK != sqlite3_exec(db, buffer, NULL, NULL, &zErrMsg)) {
        print_message(MSG_ERROR, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return DB_ERROR;
    }
    return DB_SUCCESS;
}

static uint32_t get_last_path_map_id() {
    sqlite3_stmt *res;
    const char *tail;
    if(sqlite3_prepare_v2(db, get_max_path_id_cmd, -1, &res, &tail) != SQLITE_OK) {
        print_message(MSG_ERROR, "Cannot get last path_map id\n");
    } else {
        const char * tmp;
        sqlite3_step(res);
        tmp = sqlite3_column_text(res, 0);
        if (tmp) {
            uint32_t id = atoi(tmp);
            print_message(MSG_DEBUG, "Last path_map id: %d\n", id);
            return id;
        }
    }
    return 0;
}

static uint32_t get_path_id(const char * path) {
    sqlite3_stmt *res;
    const char *tail;
    char buffer[sizeof(get_path_id_cmd) + PATH_MAX] = {0};
    sprintf(buffer, get_path_id_cmd, path);
    if(sqlite3_prepare_v2(db, buffer, -1, &res, &tail) != SQLITE_OK) {
        print_message(MSG_ERROR, "Cannot find id for path %s\n", path);
    } else {
        const char * tmp;
        sqlite3_step(res);
        tmp = sqlite3_column_text(res, 0);
        if (tmp) {
            uint32_t id = atoi(tmp);
            print_message(MSG_DEBUG, "Path id: %d\n", id);
            return id;
        } else {
            print_message(MSG_ERROR, "Cannot find id for path %s\n", path);
        }
    }
    return 0;
}
