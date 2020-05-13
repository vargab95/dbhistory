#pragma once

typedef enum
{
    MSG_TRACE = 0,
    MSG_DEBUG,
    MSG_INFO,
    MSG_WARNING,
    MSG_ERROR
} print_priority_t;

extern void print_message(print_priority_t priority, const char *format, ...);
