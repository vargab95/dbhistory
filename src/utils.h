#pragma once

typedef enum {
    MSG_TRACE = 0,
    MSG_DEBUG,
    MSG_INFO,
    MSG_WARNING,
    MSG_ERROR
} PrintPriority;

extern void print_message(PrintPriority priority, const char * format, ...);
