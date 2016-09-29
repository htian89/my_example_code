#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <linux/netlink.h>

#include <types.h>
#include <vaccess_api.h>
#include <monitor.h>

#define LISTENER_PORT 5257

int setnonblocking(int fd)
{
LLOG("");
    int opts;
    opts=fcntl(fd,F_GETFL);
    if(opts)
    {
        return -1;
    }

    opts = opts|O_NONBLOCK;

    if(fcntl(fd,F_SETFL,opts))
    {
        LLOG_ERR("%s", strerror(errno));
        return -1;
    }
    return 0;
}

int add_to_epoll(int fd, int events)
{
LLOG("");
    struct epoll_event ev = {0,{0}};
    ev.events = events;
    ev.data.fd = fd;
    if(epoll_ctl(controller->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1){
        LLOG_ERR("epoll_ctl: conn_sock");
        return -1;
    }
    return 0;
}

void del_from_epoll(int fd)
{
    struct epoll_event ev = {0,{0}};
    ev.data.fd = fd;
    if(epoll_ctl(controller->epoll_fd, EPOLL_CTL_DEL, fd, &ev) == -1){
        LLOG_ERR("epoll_ctl: conn_sock");
    }
}

int _send(int fd, char *out, int size)
{
    int ret; 
    ret = send(fd, out, size, 0);
    if(ret != size){
        LLOG_ERR("send failed");
        del_from_epoll(fd);
        return -1;
    }    
    return 0;
}

gboolean listener_init()
{
    const int val = 1;
    int ret;
    int sockfd;
    struct sockaddr_in server_addr;
    int server_addr_len;
    if((sockfd= socket(AF_INET,SOCK_STREAM,0))< 0){ 
        LLOG_ERR("create sockfd_usbip_server error!\n");
        return 1;
    }   
                               
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (ret < 0)
    {
        LLOG_ERR("setsockopt: SO_REUSEADDR\n");
        return FALSE;
    }
                               
    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
    if (ret < 0)
    {
        LLOG_ERR("setsockopt: TCP_NODELAY\n");
        return FALSE;
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(LISTENER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr_len = sizeof(server_addr);
    bind(sockfd, (struct sockaddr *)&server_addr, server_addr_len);

    listen(sockfd, 5);

    controller->listener = sockfd;

    return TRUE;
}

int wait_for_connect()
{
    struct epoll_event _events[MAX_EVENTS];
    int n, ret;
    int nfds, listener, epoll_fd, conn_sock;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    listener = controller->listener;

    epoll_fd = epoll_create(MAX_EVENTS);    
    if (epoll_fd == -1) {
        LLOG_ERR("epoll_create");
        exit(EXIT_FAILURE);
    }
    controller->epoll_fd = epoll_fd;

    if(add_to_epoll(listener, EPOLLIN))
    {
        exit(EXIT_FAILURE);
    }
    
    for(;;) {
        nfds = epoll_wait(epoll_fd, _events, MAX_EVENTS, -1);
        if(nfds == -1){
            if(errno == EINTR)
                continue;

            LLOG_ERR("%s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        for(n = 0; n < nfds; n++) {
            if(_events[n].data.fd == listener) {
                conn_sock = accept(listener, (struct sockaddr *) & client_addr, &client_addr_len);
                LLOG("conn_sock = %d", conn_sock);
                if (conn_sock == -1) {
                    LLOG_ERR("%s", strerror(errno));
                    break;
                }
                if(add_to_epoll(conn_sock, EPOLLIN | EPOLLET))
                {
                    break;
                }
            } else if(_events[n].events & EPOLLIN){
                ret = recv(_events[n].data.fd, controller->recvbuf, BUFSIZE, 0);
                if(ret == 0){
                    LLOG("disconnect %s", strerror(errno));
                    del_from_epoll(_events[n].data.fd);
                    close(_events[n].data.fd);
                    break;
                } else if (ret < 0){
                    LLOG_ERR("%s", strerror(errno));
                    del_from_epoll(_events[n].data.fd);
                    close(_events[n].data.fd);
                    break;
                }
                LLOG("recv fd = %d ,ret = %d", _events[n].data.fd, ret);
                vaccess_handle(_events[n].data.fd, controller->recvbuf);
            } else if(_events[n].events & EPOLLOUT){
                LLOG("send");
            } else {
                LLOG_ERR("events = %x", _events[n].events);
            }
        }
    }
}
