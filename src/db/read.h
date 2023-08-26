#ifndef __DB_READ_H__
#define __DB_READ_H__

#include "db/common.h"

extern db_return_codes_t db_get_history(const char *path, int limit, directory_history_t *history);
extern db_return_codes_t db_get_pinnings(const char *path, directory_pinnings_t *pinnings);

#endif
