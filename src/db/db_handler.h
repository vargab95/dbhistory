#pragma once

#include <time.h>
#include "db/db_common.h"
#include "db/db_read.h"
#include "db/db_read_re.h"

extern db_return_codes_t db_connect(const char *db_path);
extern db_return_codes_t db_add_record(const char *path, const char *command);
extern db_return_codes_t db_close();
