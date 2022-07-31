#include <stdlib.h>
#include <string.h>

#include "db/sqlite_wrapper.h"
#include "db/common.h"
#include "common.h"
#include "utils.h"

static db_return_codes_t insert_path_and_get_id(const char *path, uint32_t *path_id);
static char *escape_command(const char *command);

extern db_return_codes_t db_add_record(const char *path, const char *command)
{
    db_return_codes_t return_code;
    const char insert_history_record_cmd[] = "INSERT INTO history(path_id, command) VALUES(%d, \"%s\");";
    uint32_t path_id;
    char *escaped_command, *ecptr;
    int result;

    return_code = insert_path_and_get_id(path, &path_id);
    if (DB_SUCCESS != return_code)
    {
        return return_code;
    }

    escaped_command = escape_command(command);
    result = sql_run_command(NULL, NULL, insert_history_record_cmd, path_id, escaped_command);
    free(escaped_command);

    return result;
}

extern db_return_codes_t db_pin_command(const char *path, const char *command)
{
    db_return_codes_t return_code;
    const char insert_pinning_cmd[] = "INSERT INTO pinnings(path_id, command) VALUES(%d, \"%s\");";
    uint32_t path_id;
    char *escaped_command, *ecptr;
    int result;

    return_code = insert_path_and_get_id(path, &path_id);
    if (DB_SUCCESS != return_code)
    {
        return return_code;
    }

    escaped_command = escape_command(command);
    result = sql_run_command(NULL, NULL, insert_pinning_cmd, path_id, escaped_command);
    free(escaped_command);

    return result;
}

static db_return_codes_t insert_path_and_get_id(const char *path, uint32_t *path_id)
{
    const char insert_path_cmd[] = "INSERT INTO path_map(path) VALUES (\"%s\");";

    if (DB_SUCCESS == sql_run_command(NULL, NULL, insert_path_cmd, path))
    {
        print_message(MSG_DEBUG, "Using last row insert id.\n");
        *path_id = sql_get_last_insertion_id();
    }
    else
    {
        *path_id = get_path_id(path);
        if (0 == *path_id)
        {
            print_message(MSG_ERROR, "Systematic software failure. Unknown path id after insert operation.\n");
            return DB_ERROR;
        }
    }

    return DB_SUCCESS;
}

static char *escape_command(const char *command)
{
    char *escaped_command, *ecptr;

    ecptr = escaped_command = malloc(strlen(command) * 2);
    for (const char *cptr = command; *cptr; ++cptr)
    {
        switch (*cptr)
        {
        case '"':
            *ecptr = '"';
            ++ecptr;
            *ecptr = '"';
            ++ecptr;
            break;
        default:
            *ecptr = *cptr;
            ++ecptr;
            break;
        }
    }
    *ecptr = '\0';

    return escaped_command;
}
