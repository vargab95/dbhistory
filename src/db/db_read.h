#ifndef __DB_READ_H__
#define __DB_READ_H__

#include "db/db_common.h"

extern db_return_codes_t db_get_history(const char *path, directory_history_t *history);

#endif
