#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <regex.h>

#define __USE_XOPEN
#include <time.h>

#include "utils.h"
#include "sqlite_wrapper.h"
#include "db/db_common.h"
#include "db/db_read.h"

typedef struct
{
    regex_t regex;
    int last_id;
    int *path_id_list;
} path_filter_t;

static int get_records_callback(void *data, int argc, char **argv, char **col_names);
static int get_record_count_callback(void *data, int argc, char **argv, char **col_names);
static int get_path_count_callback(void *data, int argc, char **argv, char **col_names);
static int get_path_callback(void *data, int argc, char **argv, char **col_names);

static int get_records_callback(void *data, int argc, char **argv, char **col_names)
{
    directory_history_t *history = data;
    print_message(MSG_TRACE, "RE record: %s %s %s\n", argv[0], argv[1], argv[2]);
    copy_history_record(&history->records[history->length], argv[0], argv[1], argv[2]);
    history->length++;
    return 0;
}

static int get_record_count_callback(void *data, int argc, char **argv, char **col_names)
{
    int *record_cnt_ptr = (int *)data;
    *record_cnt_ptr = atoi(argv[0]);
    print_message(MSG_DEBUG, "Record count: %d\n", *record_cnt_ptr);
    return 0;
}

static int get_path_count_callback(void *data, int argc, char **argv, char **col_names)
{
    int *path_cnt_ptr = (int *)data;
    *path_cnt_ptr = atoi(argv[0]);
    print_message(MSG_DEBUG, "Path count: %d\n", *path_cnt_ptr);
    return 0;
}

static int get_path_callback(void *data, int argc, char **argv, char **col_names)
{
    print_message(MSG_TRACE, "Path %s %s\n", argv[0], argv[1]);
    path_filter_t *filter = data;
    if (0 == regexec(&filter->regex, argv[1], 0, NULL, 0))
    {
        print_message(MSG_DEBUG, "Path %s was matched.\n", argv[1]);
        filter->path_id_list[filter->last_id] = atoi(argv[0]);
        filter->last_id++;
    }
    return 0;
}

extern DBReturnCodes db_search_history(const char *path, directory_history_t *history)
{
    const char get_path_count_cmd[] = "SELECT COUNT(*) FROM path_map;";
    const char get_path_cmd[] = "SELECT * FROM path_map;";
    char *get_records_cmd;
    char *get_record_count_cmd;
    int path_cnt = -1;
    int record_cnt = -1;
    path_filter_t filter;
    print_message(MSG_TRACE, "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

    if (SQLITE_OK != sql_run_command(get_path_count_callback, &path_cnt, get_path_count_cmd, NULL))
    {
        print_message(MSG_ERROR, "Path table reading was unsuccessful.");
        return DB_ERROR;
    }

    if (0 >= path_cnt)
    {
        return DB_EMPTY;
    }

    filter.path_id_list = (int *)malloc(sizeof(int *) * path_cnt);
    filter.last_id = 0;
    if (regcomp(&filter.regex, path, REG_EXTENDED | REG_NOSUB) != 0)
    {
        print_message(MSG_ERROR, "Invalid regexp: %s\n", path);
        return DB_ERROR;
    }

    if (SQLITE_OK != sql_run_command(get_path_callback, &filter, get_path_cmd, NULL))
    {
        print_message(MSG_ERROR, "Error while getting matching pathes\n");
        return DB_ERROR;
    }
    regfree(&filter.regex);

    if (filter.last_id <= 0)
    {
        print_message(MSG_ERROR, "No matching path.\n");
        return DB_EMPTY;
    }

    // TODO Shall be refactored
    get_records_cmd = (char *)malloc(sizeof(char) * 10 * path_cnt + 1000);
    get_record_count_cmd = (char *)malloc(sizeof(char) * 10 * path_cnt + 500);
    if (filter.last_id > 1)
    {
        sprintf(get_records_cmd, "SELECT path, command, timestamp FROM history INNER JOIN path_map ON path_map.id = history.path_id WHERE path_id IN (%d", filter.path_id_list[0]);
        sprintf(get_record_count_cmd, "SELECT count(*) FROM history WHERE path_id IN (%d", filter.path_id_list[0]);
        for (int i = 1; i < filter.last_id - 2; ++i)
        {
            sprintf(get_records_cmd, "%s, %d", get_records_cmd, filter.path_id_list[i]);
            sprintf(get_record_count_cmd, "%s, %d", get_record_count_cmd, filter.path_id_list[i]);
        }
        sprintf(get_records_cmd, "%s, %d);", get_records_cmd, filter.path_id_list[filter.last_id - 1]);
        sprintf(get_record_count_cmd, "%s, %d);", get_record_count_cmd, filter.path_id_list[filter.last_id - 1]);
    }
    else
    {
        sprintf(get_records_cmd, "SELECT path, command, timestamp FROM history INNER JOIN path_map ON path_map.id = history.path_id WHERE path_id IN (%d);", filter.path_id_list[0]);
        sprintf(get_record_count_cmd, "SELECT count(*) FROM history WHERE path_id IN (%d);", filter.path_id_list[0]);
    }
    print_message(MSG_DEBUG, "Regex based record list command: %s\n", get_records_cmd);
    free(filter.path_id_list);

    if (SQLITE_OK != sql_run_command(get_record_count_callback, &record_cnt, get_record_count_cmd, NULL))
    {
        print_message(MSG_ERROR, "Error while fetching record count.\n");
        return DB_ERROR;
    }

    history->records = (history_record_t *)malloc(sizeof(history_record_t) * record_cnt);
    history->length = 0;

    if (SQLITE_OK != sql_run_command(get_records_callback, history, get_records_cmd, NULL))
    {
        print_message(MSG_ERROR, "Error while fetching records.\n");
        return DB_ERROR;
    }
    free(get_records_cmd);

    return DB_SUCCESS;
}
