#include <stdlib.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>

#include <linux/limits.h>
#include <sqlite3.h>

#include "utils.h"
#include "sqlite_wrapper.h"
#include "common.h"

static int get_path_id_callback(void *data, int argc, char **argv, char **col_names);
static int get_record_count_callback(void *data, int argc, char **argv, char **col_names);

extern int get_record_count(int path_id)
{
    const char count_records_cmd[] = "SELECT count(*) FROM history WHERE path_id = %d;";
    int record_cnt = 0;
    if (SQLITE_OK != sql_run_command(get_record_count_callback, &record_cnt, count_records_cmd, path_id))
    {
        return -1;
    }
    return record_cnt;
}

extern uint32_t get_path_id(const char *path)
{
    const char get_path_id_cmd[] = "SELECT * FROM path_map WHERE path = \"%s\" LIMIT 1;";
    char buffer[sizeof(get_path_id_cmd) + PATH_MAX] = {0};
    int path_id = 0;
    if (sql_run_command(get_path_id_callback, &path_id, get_path_id_cmd, path) != SQLITE_OK)
    {
        print_message(MSG_ERROR, "Cannot find id for path %s\n", path);
    }
    return path_id;
}

static int get_record_count_callback(void *data, int argc, char **argv, char **col_names)
{
    int *path_id_ptr = (int *)data;
    *path_id_ptr = atoi(argv[0]);
    print_message(MSG_DEBUG, "Record count: %d\n", *path_id_ptr);
    return 0;
}

static int get_path_id_callback(void *data, int argc, char **argv, char **col_names)
{
    int *path_id_ptr = (int *)data;
    if (!data)
    {
        print_message(MSG_ERROR, "Null ptr was received in %s\n", __FUNCTION__);
        return 1;
    }
    *path_id_ptr = atoi(argv[0]);
    print_message(MSG_DEBUG, "Path id: %d\n", *path_id_ptr);
    return 0;
}

extern void copy_history_record(history_record_t *record,
                                const char *path,
                                const char *command,
                                const char *time)
{
    size_t str_size;
    struct tm tm;

    str_size = strlen(path) + 1;
    record->path = (char *)malloc(str_size * sizeof(char));
    memcpy((char *const)record->path, path, str_size);

    str_size = strlen(command) + 1;
    record->command = (char *)malloc(str_size * sizeof(char));
    memcpy((char *const)record->command, command, str_size);

    strptime(time, "%Y-%m-%d %H:%M:%S", &tm);
    record->timestamp = mktime(&tm);
}
