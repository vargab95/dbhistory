#pragma once

typedef enum {
    MSG_TRACE,
    MSG_DEBUG,
    MSG_INFO = 0,
    MSG_WARNING,
    MSG_ERROR
} PrintPriority;

extern void print_message(PrintPriority priority, const char * format, ...);
