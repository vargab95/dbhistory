#ifndef __CLIENT_H__
#define __CLIENT_H__

typedef enum
{
    CL_ERROR = -1,
    CL_OK = 0,
    CL_EMPTY,
    CL_INVALID_INPUT,
    CL_INVALID_PATH
} client_return_codes_t;

extern client_return_codes_t client_add_record(const char *command);
extern client_return_codes_t client_pin_command(const char *command);
extern client_return_codes_t client_unpin_command(unsigned int id);
extern client_return_codes_t client_get_records(const char *path, unsigned char use_pinnings);
extern client_return_codes_t client_search_records(const char *pattern, unsigned char use_pinnings);

#endif
