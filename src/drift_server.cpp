#include <csignal>
#include <cstring>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "dconf.h"
#include "log.h"
#include "daemon.h"
#include "drift_server.h"

typedef struct
{
    int signo;
    void  (*handler)(int signo);
}signal_t;

static volatile sig_atomic_t g_stop = 0;
static volatile sig_atomic_t g_restart = 1; // 第一次启动时restart为1，表示线程需要启动。

bool stop_process()
{
    return (g_stop == 1);
}

bool restart_all_threads()
{
    return (g_restart == 1);
}

static inline void reset_restart_flag()
{
    g_restart = 0;
}

static void signal_handler(int signo)
{
    switch (signo)
    {
        case SIGINT:
            g_stop = 1;
            break;
        case SIGTERM:
            g_stop = 1;
            break;
        case SIGUSR1:
            g_restart = 1;
            break;
    }
}

static signal_t signals[] = 
{
    {SIGINT, signal_handler},
    {SIGTERM, signal_handler},
    {SIGUSR1, signal_handler},
    {SIGPIPE, SIG_IGN},
    {SIGHUP, SIG_IGN},
    {SIGCHLD, SIG_IGN},
    {SIGQUIT, SIG_IGN},
    {0, NULL}
};

static int setup_signal_handlers()
{
    signal_t      *sig;
    struct sigaction   sa;

    for (sig = signals; sig->signo != 0; sig++) {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = sig->handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(sig->signo, &sa, NULL) == -1) {
            return -1;
        }
    }

    return 0;
}

static void init_log()
{
    const char* str = "drift-server.log";
    LOGGER.set_file_name(CONFIG.get("log-file", str));

    str = "INFO";
    LOGGER.set_log_level(CONFIG.get("log-level", str));

    LOGGER.set_max_file_size(CONFIG.get("log-max-size", 64 * 1024 * 1024));
}

static bool init_config(const char* config_path)
{
    return CONFIG.load(config_path, true);
}

static int set_rlimit()
{
    struct rlimit rl = {0,0};
    int ret = 0;

    do
    {
        if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
        {
            ret = -1;
            break;
        }
        rl.rlim_cur = rl.rlim_max;
        if (setrlimit(RLIMIT_NOFILE, &rl) != 0 )
        {
            ret = -1;
            break;
        }

        if (getrlimit(RLIMIT_CORE, &rl) != 0)
        {
            ret = -1;
            break;
        }

        rl.rlim_cur = rl.rlim_max;
        if (setrlimit(RLIMIT_CORE, &rl) != 0) 
        {
            ret = -1;
            break;
        }
    }
    while(0);

    return ret;
}

static int check_single()
{
    const char* str = CONFIG.get("daemon-lock", "daemon.pid");

    if(LibSys::daemon_lock(str) < 0)
        return -1;

    return 0;
}

static inline void print_usage()
{
    printf("usage: drift-server <config-file>\n");
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        print_usage();
        return 1;
    }

    if(!init_config(argv[1]))
    {
        // log module is not inited, so use fprintf.
        fprintf(stderr, "\tread configurations from %s failed, exiting...\n", argv[1]);
        return 1;
    }

    init_log();

    if(setup_signal_handlers() < 0)
    {
        LOG(ERROR, "setup signal handlers failed, exiting..."); 
        return 1;
    }

    if(set_rlimit() < 0)
    {
        LOG(ERROR, "get/set rlimit failed, exiting...");
        return 1;
    }

    // 需要产生core文件，所以不需要改变路径。
    // LOGGER默认会将0，1，2定位到日志里面，所以这里不需要关闭0,1,2。
    if(LibSys::daemonize(1, 1) < 0)
    {
        LOG(ERROR, "daemonize failed, exiting...");
        return 1;
    }

    if(check_single() < 0)
    {
        LOG(ERROR, "another process already running, exiting...");
        return 2;
    }

    sigset_t sigs;
    sigemptyset(&sigs);

    LOG(INFO, "main thread starting...");

    do
    {
        if(restart_all_threads())
        {
            // 启动所有工作线程
        }

        reset_restart_flag();

        sigsuspend(&sigs);

        if(restart_all_threads() || stop_process())
        {
            // 关闭所有工作线程
        }
    }
    while(!stop_process());

    LOG(INFO, "main thread exited.");
    return 0;
}
