/*
 * websocket_client.cpp
 *
 *  Created on: 2015年1月27日
 *      Author: root
 */

#if __cplusplus < 201103L

#include "websocket_client.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

void websocket_client::connect(std::string &ip, ushort_t port)
{
    m_sockfd = socket_connect(ip, port);
    if(m_sockfd == -1)
        throw vclient_exception("connect to websocket server failed!");

    m_handle = ws_connection(m_sockfd, ip.c_str(), port);
    if(m_handle == 0)
        throw vclient_exception("connect to websocket server failed!");

    m_connected = true;
}

int websocket_client::socket_connect(std::string &ip, ushort_t port)
{
    int sockfd;
    sockaddr_in addr;
    int on = 1, ret = 0;

    sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd <= 0)
    {
        LOG_ERR("create sock failed!");
        return -1;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret < 0)
    {
        LOG_ERR("setsockopt to set SO_REUSEADDR error");
        return -1;
    }

    struct timeval timeout = {10,0};
    ret = ::setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
    if (ret < 0)
    {
        LOG_ERR("setsockopt to set SO_RCVTIMEO error");
        return -1;
    }

    if(::connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
    {
        LOG_ERR("connet server failed!");
        return -1;
    }

    return sockfd;
}

const std::size_t websocket_client::received_size()
{
    return m_recv_size;
}

const char *websocket_client::receive()
{
    m_recv_size = ws_recv(m_handle, m_recv_buf, BUF_SIZE);
    if(m_recv_size <=0)
    {
        LOG_ERR("receive from websocket server failed!");
        throw vclient_exception("receive from websocket server failed!");
    }
    m_recv_buf[m_recv_size] = '\0';
    return m_recv_buf;
}

void websocket_client::send(std::string &msg)
{
    if(m_connected == false)
    {
        LOG_ERR("websocket server disconnected");
        throw vclient_exception("websocket server disconnected!");
    }

    if(ws_send(m_handle, msg.c_str(), msg.length()) < 0)
        throw vclient_exception("send to websocket server failed!");
}

void websocket_client::close()
{
    if(m_connected)
    {
        ::close(m_sockfd);
        m_connected = false;
    }
}

#endif
