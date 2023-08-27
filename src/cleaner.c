#include <unistd.h>

#define __USE_XOPEN
#include <time.h>

#include "cleaner.h"
#include "config.h"
#include "utils.h"

#include "db/handler.h"
#include "db/path.h"
#include "db/sqlite_wrapper.h"

static void check_non_referenced_path(const char *path);
static void check_non_existing_path(const char *path);
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

    delete_old_records();

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
        if (DB_SUCCESS != sql_run_command(NULL, NULL, "DELETE FROM path_map WHERE id=%d;", path_id))
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
        if (DB_SUCCESS != sql_run_command(NULL, NULL, "DELETE FROM history WHERE path_id=%d;", get_path_id(path)))
        {
            print_message(MSG_ERROR, "Could not delete records with non-existing path %s\n", path);
            return;
        }
        print_message(MSG_ERROR, "Records with non-existing path %s were deleted successfully\n", path);

        if (DB_SUCCESS != sql_run_command(NULL, NULL, "DELETE FROM path_map WHERE id=%d;", get_path_id(path)))
        {
            print_message(MSG_ERROR, "Could not delete non-existing path %s\n", path);
            return;
        }
        print_message(MSG_ERROR, "Non-existing path %s was deleted successfully\n", path);
    }
}

static void delete_old_records()
{
    const char delete_records_cmd[] = "DELETE FROM history WHERE timestamp < \"%s\";";

    if (g_dbhistory_configuration.deletion_time_threshold < 0)
    {
        print_message(MSG_INFO, "Old record deletion is disabled\n");
        return;
    }

    time_t now = time(0);
    const char buffer[128];
    time_t deletion_threshold = now - g_dbhistory_configuration.deletion_time_threshold;
    struct tm *deletion_threshold_tm = localtime(&deletion_threshold);

    strftime((char*)buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", deletion_threshold_tm);

    if (DB_SUCCESS != sql_run_command(NULL, NULL, delete_records_cmd, buffer))
    {
        print_message(MSG_ERROR, "Deleting all old records was failed.\n");
        return;
    }
}
