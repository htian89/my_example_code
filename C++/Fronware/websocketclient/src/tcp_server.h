/*
 * tcp_server.h
 *
 *  Created on: 2014年12月23日
 *      Author: root
 */

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <boost/thread/mutex.hpp>

#include "common/utils.h"
#include "common/io_service_pool.hpp"

#include "tcp_session.h"
#include "websocket_interface.h"

class tcp_server
{
public:
    typedef io_service_pool::ios_type ios_type;

    tcp_server(ushort_t port, int n = 4);
    tcp_server(io_service_pool &ios, ushort port);
    void start();
    void run();

    void send_to_vclient(const uint32_t msg_type);

    websocket_interface *get_websocket_interface() {return &m_ws_interface;};

private:
    io_service_pool &m_ios_pool;
    typedef boost::asio::ip::tcp::acceptor acceptor_type;
    acceptor_type m_acceptor;
    websocket_interface m_ws_interface;
    boost::mutex m_mutex;
    tcp_session_ptr m_vclient;

    void start_accept();
    void handle_accept(const boost::system::error_code &error,
            tcp_session_ptr session);
};

#endif /* TCP_SERVER_H_ */
