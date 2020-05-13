#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define __USE_XOPEN
#include <time.h>

#include "utils.h"
#include "sqlite_wrapper.h"
#include "db/common.h"
#include "db/path.h"

static int get_pathes_callback(void *data, int argc, char **argv, char **col_names);
static int get_path_count_callback(void *data, int argc, char **argv, char **col_names);

extern int db_get_path_count()
{
    const char count_pathes_cmd[] = "SELECT count(*) FROM path_map;";
    int path_count = 0;
    if (SQLITE_OK != sql_run_command(get_path_count_callback, &path_count, count_pathes_cmd))
    {
        return -1;
    }
    return path_count;
}

extern db_return_codes_t db_get_pathes(char ***pathes, int *count)
{
    print_message(MSG_TRACE, "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

    const char get_pathes_cmd[] = "SELECT path FROM path_map;";
    char **tmp_pathes;
    *count = db_get_path_count();

    if (0 >= *count)
    {
        return DB_EMPTY;
    }

    *pathes = (char **)malloc(sizeof(char *) * *count);
    tmp_pathes = *pathes;

    if (SQLITE_OK != sql_run_command(get_pathes_callback, &tmp_pathes, get_pathes_cmd))
    {
        return DB_ERROR;
    }

    return DB_SUCCESS;
}

static int get_pathes_callback(void *data, int argc, char **argv, char **col_names)
{
    char ***path = data;
    **path = strdup(argv[0]);
    (*path)++;
    return 0;
}

static int get_path_count_callback(void *data, int argc, char **argv, char **col_names)
{
    int *path_count_ptr = (int *)data;
    *path_count_ptr = atoi(argv[0]);
    print_message(MSG_DEBUG, "Path count: %d\n", *path_count_ptr);
    return 0;
}