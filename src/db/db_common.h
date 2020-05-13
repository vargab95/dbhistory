#ifndef __DB_COMMON_H__
#define __DB_COMMON_H__

#include <time.h>
#include <stdint.h>

typedef enum
{
    DB_ERROR = 0,
    DB_SUCCESS,
    DB_EMPTY
} db_return_codes_t;

typedef struct
{
    const char *path;
    const char *command;
    time_t timestamp;
} history_record_t;

typedef struct
{
    history_record_t *records;
    unsigned int length;
} directory_history_t;

extern uint32_t get_path_id(const char *path);
extern int get_record_count(int path_id);
extern void copy_history_record(history_record_t *record,
                                const char *path,
                                const char *command,
                                const char *time);

#endif
