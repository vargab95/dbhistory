#pragma once

#include <time.h>

typedef enum {
    DB_ERROR = 0,
    DB_SUCCESS,
    DB_EMPTY
} DBReturnCodes;

typedef struct {
    const char * path;
    const char * command;
    time_t timestamp;
} history_record_t;

typedef struct {
    history_record_t * records;
    unsigned int length;
} directory_history_t;

extern DBReturnCodes db_connect(const char * db_path);
extern DBReturnCodes db_add_record(const char * path, const char * command);
extern DBReturnCodes db_get_history(const char * path, directory_history_t * history);
extern DBReturnCodes db_close();
