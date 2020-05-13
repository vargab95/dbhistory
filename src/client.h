#ifndef __CLIENT_H__
#define __CLIENT_H__

typedef enum
{
    CL_MISSING_DAEMON,
    CL_ERROR,
    CL_OK = 0,
    CL_EMPTY,
    CL_INVALID_INPUT,
    CL_INVALID_PATH
} client_return_codes_t;

extern client_return_codes_t client_add_record(const char *command);
extern client_return_codes_t client_get_records(const char *path);
extern client_return_codes_t client_search_records(const char *pattern);

#endif