#include <types.h>
#include <ev_epoll.h>

#define MAX_MONITORS 6
Monitor *monitor;

gboolean monitor_init(void)
{
    GdkRectangle *geometry;
    monitor = calloc(MAX_MONITORS, sizeof(Monitor));
    int i = 0;
    for(; i < controller->monitor_num; i++)
    {
        monitor[i].number = i+1;
        monitor[i].status = FREE_MONITOR;
        geometry = &monitor[i].geometry;
        gdk_screen_get_monitor_geometry(controller->screen, i, geometry);
        g_debug("x = %d, y = %d, width = %d, height = %d", geometry->x, geometry->y, geometry->width, geometry->height);
    }
}

void monitor_destroy(void)
{
    int n;

    if(monitor == NULL)
        return;

    for(n = 0; n < controller->monitor_num; n++)
    {
        if(monitor[n].status == USED_MONITOR && monitor[n].pid > 0)
        {
            kill(monitor[n].pid, SIGINT);
        }
    }
    free(monitor);
    monitor = NULL;
}

int use_monitor(int n)
{
    pid_t pid;
    monitor[n-1].status = USED_MONITOR;

    pid = vfork();
    switch(pid)
    {
    case -1:
        LLOG_ERR("fork faild");
        monitor[n-1].status = FREE_MONITOR;
        return -1;
    case 0:
        switch(monitor[n-1].mode)
        {
        case CONNECT_BY_FAP:
        {
            char (*cmd)[24] = calloc(3, 24);
            sprintf(cmd[0], "%d", monitor[n-1].targetPort);
            strcpy(cmd[1], monitor[n-1].targetIp);
            sprintf(cmd[2], "%dx%d", monitor[n-1].geometry.x, monitor[n-1].geometry.y);
            LLOG("%s %s %s",cmd[0],cmd[1],cmd[2]);
            execlp("/usr/bin/fap", "fap", "-p", cmd[0],"-h", cmd[1], "--position", cmd[2],"-f", NULL);
            break;
        }
        case CONNECT_BY_VNC:
        {
            char (*cmd)[24] = calloc(3, 24);
            sprintf(cmd[0], "%s:%d", monitor[n-1].targetIp, monitor[n-1].targetPort);
            sprintf(cmd[1], "%dx%d", monitor[n-1].geometry.x, monitor[n-1].geometry.y);
            sprintf(cmd[2], "%dx%d", monitor[n-1].geometry.width, monitor[n-1].geometry.height);
            LLOG("%s %s %s",cmd[0],cmd[1],cmd[2]);
            execlp("/usr/bin/vncviewer", "vncviewer", cmd[0], "-position", cmd[1], "-geometry", cmd[2], "-fullscreen", "-passwd", "/opt/vncviewer/passwd", NULL);
            break;
        }
        case CONNECT_BY_CAMERA:
            monitor[n-1].status = FREE_MONITOR;
            break;
        default:
            break;
        }
        break;
    default:
        monitor[n-1].pid = pid;
        LLOG("use monitor%d pid = %d", n, pid);
        break;
    }
    return 0;
}

int free_monitor(int n)
{
    LLOG("");
    monitor[n-1].status = FREE_MONITOR;
    monitor[n-1].pid = 0;
    return 0;
}

int find_monitor_by_pid(unsigned int pid)
{
    int i;
    for(i = 0; i < MAX_MONITORS; i++)
    {
        if(monitor[i].pid == pid)
        {
            LLOG("find monitor%d pid = %d", i+1, pid);
            return i+1;
        }
    }
    return 0;
}

static void gtk_win_destroy(GtkWidget *widget,gpointer data)
{
    g_debug("exit\n");
    gtk_main_quit();
}

void monitor_test(GdkScreen *screen)
{
    gint n_monitors = gdk_screen_get_n_monitors(screen);
    g_debug("number of monitors = %d \n", n_monitors);

    int i = 0;
    for(; i < n_monitors; i++)
    {
        GtkWidget *window; //Widget对象指针声明
        GdkRectangle monitor;
        gdk_screen_get_monitor_geometry(screen, i, &monitor);
        g_debug("x = %d, y = %d, width = %d, height = %d", monitor.x, monitor.y, monitor.width, monitor.height);

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL); //创建窗口对象

        g_signal_connect(GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(gtk_win_destroy),NULL);

        gtk_window_set_default_size(GTK_WINDOW(window), monitor.width, monitor.height);
        gtk_window_move(GTK_WINDOW(window), monitor.x, monitor.y);
        gtk_window_set_title(GTK_WINDOW(window), "test");
    //    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    //    gtk_window_set_screen(GTK_WINDOW(window), screen);

        gtk_widget_show_all(window); //显示
    }

    gtk_main();
}
