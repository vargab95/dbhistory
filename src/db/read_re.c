#include <regex.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>

#include "db/common.h"
#include "db/read.h"
#include "sqlite_wrapper.h"
#include "utils.h"

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

extern db_return_codes_t db_search_history(const char *path, directory_history_t *history)
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

    filter.path_id_list = (int *)malloc(sizeof(int) * path_cnt);
    filter.last_id = 0;
    if (regcomp(&filter.regex, path, REG_EXTENDED | REG_NOSUB) != 0)
    {
        print_message(MSG_ERROR, "Invalid regexp: %s\n", path);
        free(filter.path_id_list);
        return DB_ERROR;
    }

    if (SQLITE_OK != sql_run_command(get_path_callback, &filter, get_path_cmd, NULL))
    {
        print_message(MSG_ERROR, "Error while getting matching pathes\n");
        free(filter.path_id_list);
        return DB_ERROR;
    }
    regfree(&filter.regex);

    if (filter.last_id <= 0)
    {
        print_message(MSG_ERROR, "No matching path.\n");
        free(filter.path_id_list);
        return DB_EMPTY;
    }

    if (filter.last_id > 1)
    {
        const char *command_format = "SELECT path, command, timestamp FROM history "
                                     "INNER JOIN path_map ON path_map.id = history.path_id "
                                     "WHERE path_id IN (";
        const char *count_command_format = "SELECT count(*) FROM history WHERE path_id IN (";
        char path_id_buffer[32];

        get_records_cmd = (char *)malloc(sizeof(char) * 12 * path_cnt + strlen(command_format) + 3);
        get_record_count_cmd = (char *)malloc(sizeof(char) * 12 * path_cnt + strlen(count_command_format) + 3);

        strcpy(get_records_cmd, command_format);
        strcpy(get_record_count_cmd, count_command_format);

        for (int i = 0; i < filter.last_id - 1; ++i)
        {
            sprintf(path_id_buffer, (i == 0) ? "%10d" : ", %10d", filter.path_id_list[i]);
            strcat(get_records_cmd, path_id_buffer);
            strcat(get_record_count_cmd, path_id_buffer);
        }

        strcat(get_records_cmd, ");");
        strcat(get_record_count_cmd, ");");
    }
    else
    {
        const char *get_records_command_format = "SELECT path, command, timestamp FROM history "
                                                 "INNER JOIN path_map ON path_map.id = history.path_id "
                                                 "WHERE path_id IN (%10d);";
        const char *get_records_command_count_format = "SELECT count(*) FROM history WHERE path_id IN (%10d);";

        get_records_cmd = (char *)malloc(sizeof(char) * strlen(get_records_command_format) + 10);
        get_record_count_cmd = (char *)malloc(sizeof(char) * strlen(get_records_command_count_format) + 10);

        sprintf(get_records_cmd, get_records_command_format, filter.path_id_list[0]);
        sprintf(get_record_count_cmd, get_records_command_count_format, filter.path_id_list[0]);
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
