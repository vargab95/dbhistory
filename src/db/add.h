#ifndef __DB_ADD_H__
#define __DB_ADD_H__

#include "db/common.h"

extern db_return_codes_t db_add_record(const char *path, const char *command);
extern db_return_codes_t db_pin_command(const char *path, const char *command);

#endif
