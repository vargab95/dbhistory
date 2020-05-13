#include <unistd.h>
#include <signal.h>

#include "config.h"
#include "daemon.h"
#include "utils.h"

static enum {
    DAEMON_INIT = 0,
    DAEMON_RUN,
    DAEMON_STOP
} daemon_state = DAEMON_INIT;

void graceful_stop(int sig);

extern void daemon_run()
{
    print_message(MSG_TRACE, "Initializing signal handlers\n");
    signal(SIGTERM, graceful_stop);
    signal(SIGINT, graceful_stop);

    daemon_state = DAEMON_RUN;
    while (DAEMON_RUN == daemon_state)
    {
        print_message(MSG_TRACE, "Daemon sys tick\n");
        sleep(g_dbhistory_configuration.daemon_tick_time);
    }
}

void graceful_stop(int sig)
{
    switch (sig)
    {
    case SIGINT:
        print_message(MSG_INFO, "User interrupt caught. Terminating ...\n");
        break;
    case SIGTERM:
        print_message(MSG_INFO, "Termination signal caught. Terminating ...\n");
        break;
    default:
        print_message(MSG_ERROR, "Unhandled signal\n");
    }
    daemon_state = DAEMON_STOP;
}
