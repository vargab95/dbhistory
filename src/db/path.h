#ifndef __DB_PATH_H__
#define __DB_PATH_H__

#include "db/common.h"

extern int db_get_path_count();
extern db_return_codes_t db_get_pathes(char ***pathes, int *count);

#endif
