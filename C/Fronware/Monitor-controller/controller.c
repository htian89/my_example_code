#include <types.h>

Controller *controller = NULL;

gboolean controller_init(void)
{
    controller = calloc(1,sizeof(Controller));
    controller->screen = gdk_screen_get_default();

    controller->monitor_num = gdk_screen_get_n_monitors(controller->screen);
    g_debug("number of monitors = %d", controller->monitor_num);
    
    return TRUE;
}

static void controller_destroy(void)
{
    if(controller == NULL)
        return;
    free(controller);
    controller = NULL;
}

static void sigchld_handler(int sig)
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    LLOG("pid = %d, status = %d",pid, status);
    if(pid > 0)
    {
        int monitor_n;
        monitor_n = find_monitor_by_pid(pid);
        if(monitor_n)
            free_monitor(monitor_n);
    }
}

static void sigint_handler(int sig)
{
LLOG("");
    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    monitor_destroy();
    controller_destroy();
    exit(0);
}

void signal_init(void)
{
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
}

int main(int argc, char **argv)
{
    gtk_init(&argc,&argv); //初始化
    
    signal_init();

    controller_init();

    monitor_init();

    listener_init();

    wait_for_connect();

    return 0; 
}
