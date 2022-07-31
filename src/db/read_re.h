#ifndef __DB_READ_RE_H__
#define __DB_READ_RE_H__

#include "db/common.h"

extern db_return_codes_t db_search_history(const char *path, directory_history_t *history);
extern db_return_codes_t db_search_pinnings(const char *path, directory_pinnings_t *pinnings);

#endif
