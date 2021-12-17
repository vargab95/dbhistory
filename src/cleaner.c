#include <unistd.h>

#define __USE_XOPEN
#include <time.h>

#include "cleaner.h"
#include "config.h"
#include "sqlite_wrapper.h"
#include "utils.h"

#include "db/handler.h"
#include "db/path.h"

static void check_non_referenced_path(const char *path);
static void check_non_existing_path(const char *path);
static int delete_old_records_callback(void *data, int argc, char **argv, char **col_names);
static void delete_old_records();

extern void cleaner_run()
{
    char **pathes;
    int path_count;

    if (DB_SUCCESS != db_connect(g_dbhistory_configuration.database_path))
    {
        print_message(MSG_ERROR, "Cannot connect to the database!\n");
        return;
    }

    if (DB_SUCCESS != db_get_pathes(&pathes, &path_count))
    {
        print_message(MSG_ERROR, "Cannot read pathes!\n");
        return;
    }

    for (int i = 0; i < path_count; i++)
    {
        check_non_referenced_path(pathes[i]);
        check_non_existing_path(pathes[i]);
    }

    delete_old_records();

    db_close();
}

static void check_non_referenced_path(const char *path)
{
    int path_id;

    print_message(MSG_DEBUG, "Testing %s for non-referenced path\n", path);
    path_id = get_path_id(path);
    if (get_record_count(path_id) <= 0)
    {
        print_message(MSG_WARNING, "%s has no records!\n", path);
        if (SQLITE_OK != sql_run_command(NULL, NULL, "DELETE FROM path_map WHERE id=%d;", path_id))
        {
            print_message(MSG_ERROR, "Could not delete non-referenced path %s\n", path);
            return;
        }
        print_message(MSG_ERROR, "Non-referenced path %s was deleted successfully\n", path);
    }
}

static void check_non_existing_path(const char *path)
{
    print_message(MSG_DEBUG, "Testing %s for non-existing path\n", path);
    if (0 != access(path, F_OK))
    {
        print_message(MSG_WARNING, "%s does not exists or permission is denied!\n", path);
        if (SQLITE_OK != sql_run_command(NULL, NULL, "DELETE FROM history WHERE path_id=%d;", get_path_id(path)))
        {
            print_message(MSG_ERROR, "Could not delete records with non-existing path %s\n", path);
            return;
        }
        print_message(MSG_ERROR, "Records with non-existing path %s were deleted successfully\n", path);

        if (SQLITE_OK != sql_run_command(NULL, NULL, "DELETE FROM path_map WHERE id=%d;", get_path_id(path)))
        {
            print_message(MSG_ERROR, "Could not delete non-existing path %s\n", path);
            return;
        }
        print_message(MSG_ERROR, "Non-existing path %s was deleted successfully\n", path);
    }
}

static void delete_old_records()
{
    const char get_records_cmd[] = "SELECT id, timestamp FROM history;";

    if (g_dbhistory_configuration.deletion_time_threshold < 0)
    {
        print_message(MSG_INFO, "Old record deletion is disabled\n");
        return;
    }

    if (SQLITE_OK != sql_run_command(delete_old_records_callback, NULL, get_records_cmd))
    {
        print_message(MSG_ERROR, "Selecting all records for check was failed.\n");
        return;
    }
}

static int delete_old_records_callback(void *data, int argc, char **argv, char **col_names)
{
    struct tm tm;
    int record_id;
    time_t record_timestamp;
    time_t current_time;

    print_message(MSG_TRACE, "Delete old records was called.\n");
    print_message(MSG_TRACE, "argv[0]: %s, argv[1]: %s\n", argv[0], argv[1]);

    record_id = atoi(argv[0]);

    print_message(MSG_TRACE, "Try to decode timestamp\n");
    strptime(argv[1], "%Y-%m-%d %H:%M:%S", &tm);
    record_timestamp = mktime(&tm);

    print_message(MSG_TRACE, "Reading current time\n");
    time(&current_time);

    print_message(MSG_TRACE, "Testing timestamp\n");
    print_message(MSG_DEBUG, "Current time %d, threshold %d, record timestamp %d\n", current_time,
                  g_dbhistory_configuration.deletion_time_threshold, record_timestamp);
    if ((current_time - g_dbhistory_configuration.deletion_time_threshold) >= record_timestamp)
    {
        print_message(MSG_WARNING, "Record %d is out of date\n", record_id);
        if (SQLITE_OK !=
            sql_run_command(delete_old_records_callback, NULL, "DELETE FROM history WHERE id=%d;", record_id))
        {
            print_message(MSG_ERROR, "Couldn't delete record %d!\n", record_id);
            return 0;
        }

        print_message(MSG_INFO, "Record %d deleted successfully.\n", record_id);
        return 0;
    }

    return 0;
}
