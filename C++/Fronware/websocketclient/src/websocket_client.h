/*
 * websocket_client.h
 *
 *  Created on: 2015年1月27日
 *      Author: root
 */

#ifndef WEBSOCKET_CLIENT_H_
#define WEBSOCKET_CLIENT_H_

#if __cplusplus < 201103L

#include "common/utils.h"
#include "common/vclient_common.h"

#include "tcp_buffer.hpp"

class websocket_client {
public:
    typedef std::size_t size_type;

private:
    BOOST_STATIC_CONSTANT(size_type, BUF_SIZE = 65535);

public:
    void connect(std::string &ip, ushort_t port);
    void close();
    const char *receive();
    void send(std::string &msg);
    const size_type received_size();

private:
    int socket_connect(std::string &ip, ushort_t port);
    int m_sockfd;
    bool m_connected;
    int m_handle;
    char m_recv_buf[BUF_SIZE];
    size_type m_recv_size;
};

typedef struct _ws_connect_params
{
    int secure;  // if this is a secure connection
    int port;    // the port
    char *host;  // the host
    char *resource;  // the resource

    int sub_protocol_count;
    char **sub_protocol;
    char *origin;
} ws_connect_params;

extern "C"{
    int ws_connection(int socketfd, const char * ip, const ushort_t port);
    int ws_recv(int handle, char *payload, int len);
    int ws_send(int handle, const char *payload, int len);
    int ws_disconnection(int handle);
    int ws_can_recv(int handle);
}

#endif

#endif /* WEBSOCKET_CLIENT_H_ */
