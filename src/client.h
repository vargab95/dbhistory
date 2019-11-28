#pragma once

typedef enum {
    CL_MISSING_DAEMON,
    CL_ERROR,
    CL_OK = 0,
    CL_EMPTY,
    CL_INVALID_INPUT,
    CL_INVALID_PATH
} ClientReturnCodes;

extern ClientReturnCodes client_add_record(const char * command);
extern ClientReturnCodes client_get_records(const char * path);
extern ClientReturnCodes client_search_records(const char * pattern);
