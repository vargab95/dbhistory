#include <unistd.h>
#include <signal.h>
#include <sys/inotify.h>
#include <limits.h>

#include "config.h"
#include "daemon.h"
#include "utils.h"

#include "db/handler.h"
#include "db/path.h"

static enum {
    DAEMON_INIT = 0,
    DAEMON_RUN,
    DAEMON_STOP
} daemon_state = DAEMON_INIT;

void graceful_stop(int sig);

extern void daemon_run()
{
    print_message(MSG_TRACE, "Initializing signal handlers\n");

    char **pathes = NULL;
    int path_count = 0;
    int inotifyFd, wd, j;
    char buf[1024] __attribute__((aligned(8)));
    int numRead;
    char *p;
    struct inotify_event *event;

    signal(SIGTERM, graceful_stop);
    signal(SIGINT, graceful_stop);

    if (DB_SUCCESS != db_connect(g_dbhistory_configuration.database_path))
    {
        print_message(MSG_ERROR, "Can't open the database\n");
        return;
    }

    if (DB_ERROR == db_get_pathes(&pathes, &path_count))
    {
        print_message(MSG_ERROR, "Cannot fetch pathes\n");
        return;
    }
    else
    {
        if (path_count > 0)
        {
            print_message(MSG_DEBUG, "Pathes to watch:\n");
            for (int i = 0; i < path_count; i++)
            {
                print_message(MSG_DEBUG, "    %s\n", pathes[i]);
            }
        }
    }

    inotifyFd = inotify_init();
    if (inotifyFd < 0)
    {
        return;
    }

    for (j = 1; j < path_count; j++)
    {
        wd = inotify_add_watch(inotifyFd, pathes[j], IN_MOVED_FROM | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF);
        if (wd < 0)
        {
            return;
        }

        print_message(MSG_INFO, "Watching %s using wd %d\n", pathes[j], wd);
    }

    daemon_state = DAEMON_RUN;
    // while (DAEMON_RUN == daemon_state)
    // {
    //     print_message(MSG_TRACE, "Daemon sys tick\n");
    //     sleep(g_dbhistory_configuration.daemon_tick_time);
    // }

    for (;;)
    {
        numRead = read(inotifyFd, buf, 1024);
        if (numRead <= 0)
        {
            return;
        }

        print_message(MSG_DEBUG, "Read %ld bytes from inotify fd\n", (long)numRead);

        for (p = buf; p < buf + numRead;)
        {
            event = (struct inotify_event *)p;

            if (event->mask & IN_DELETE)
                print_message(MSG_WARNING, "IN_DELETE\n");
            if (event->mask & IN_DELETE_SELF)
                print_message(MSG_WARNING, "IN_DELETE_SELF\n");
            if (event->mask & IN_MOVE_SELF)
                print_message(MSG_WARNING, "IN_MOVE_SELF\n");
            if (event->mask & IN_MOVED_FROM)
                print_message(MSG_WARNING, "IN_MOVED_FROM\n");

            p += sizeof(struct inotify_event) + event->len;
        }
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
