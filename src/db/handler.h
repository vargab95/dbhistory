#ifndef __DB_HANDLER_H__
#define __DB_HANDLER_H__

#include "db/common.h"
#include "db/read.h"
#include "db/read_re.h"
#include <time.h>

extern db_return_codes_t db_connect(const char *db_path);
extern db_return_codes_t db_add_record(const char *path, const char *command);
extern db_return_codes_t db_close();

#endif
