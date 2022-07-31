#include "db/remove.h"
#include "db/sqlite_wrapper.h"

extern db_return_codes_t db_unpin_command(unsigned int id)
{
    const char remove_pinning_cmd[] = "DELETE FROM pinnings WHERE id = %d;";

    return sql_run_command(NULL, NULL, remove_pinning_cmd, id);
}
