#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>

#include "db/common.h"
#include "db/read.h"
#include "sqlite_wrapper.h"
#include "utils.h"

static int get_history_records_callback(void *data, int argc, char **argv, char **col_names);
static int get_pinning_records_callback(void *data, int argc, char **argv, char **col_names);

extern db_return_codes_t db_get_history(const char *path, int limit, directory_history_t *history)
{
    uint32_t path_id = get_path_id(path);
    print_message(MSG_TRACE, "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    if (0 == path_id)
    {
        print_message(MSG_INFO, "There is no history record for this folder.\n");
        return DB_EMPTY;
    }
    else
    {
        const char get_records_cmd[] = "SELECT path, command, timestamp FROM history INNER JOIN path_map ON "
                                       "path_map.id = history.path_id WHERE path_id = %d ORDER BY timestamp DESC "
                                       "LIMIT %d;";
        int record_cnt = get_record_count(path_id);

        if (0 >= record_cnt)
        {
            return DB_EMPTY;
        }

        history->records = (history_record_t *)malloc(sizeof(history_record_t) * record_cnt);
        history->length = 0;

        if (DB_SUCCESS != sql_run_command(get_history_records_callback, history, get_records_cmd, path_id, limit))
        {
            return DB_ERROR;
        }
        return DB_SUCCESS;
    }
}

static int get_history_records_callback(void *data, int argc, char **argv, char **col_names)
{
    directory_history_t *history = data;
    copy_history_record(&history->records[history->length], argv[0], argv[1], argv[2]);
    history->length++;
    return 0;
}

extern db_return_codes_t db_get_pinnings(const char *path, directory_pinnings_t *pinnings)
{
    uint32_t path_id = get_path_id(path);
    print_message(MSG_TRACE, "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    if (0 == path_id)
    {
        print_message(MSG_INFO, "There are no pinnings for this folder.\n");
        return DB_EMPTY;
    }
    else
    {
        const char get_records_cmd[] = "SELECT path, command, pinnings.id as id FROM pinnings INNER JOIN path_map ON "
                                       "path_map.id = pinnings.path_id WHERE path_id = %d;";
        int record_cnt = get_record_count(path_id);

        if (0 >= record_cnt)
        {
            return DB_EMPTY;
        }

        pinnings->records = (pinning_record_t *)malloc(sizeof(pinning_record_t) * record_cnt);
        pinnings->length = 0;

        if (DB_SUCCESS != sql_run_command(get_pinning_records_callback, pinnings, get_records_cmd, path_id))
        {
            return DB_ERROR;
        }

        return DB_SUCCESS;
    }
}

static int get_pinning_records_callback(void *data, int argc, char **argv, char **col_names)
{
    directory_pinnings_t *pinnings = data;
    copy_pinning_record(&pinnings->records[pinnings->length], argv[0], argv[1], argv[2]);
    pinnings->length++;
    return 0;
}
