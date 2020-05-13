#pragma once

#include <time.h>
#include "db/db_common.h"
#include "db/db_read.h"
#include "db/db_read_re.h"

extern DBReturnCodes db_connect(const char *db_path);
extern DBReturnCodes db_add_record(const char *path, const char *command);
extern DBReturnCodes db_close();
