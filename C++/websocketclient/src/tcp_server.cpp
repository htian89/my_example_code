/*
 * tcp_server.cpp
 *
 *  Created on: 2014年12月23日
 *      Author: root
 */

#include "tcp_server.h"

using namespace boost;
using namespace boost::asio;

tcp_server::tcp_server(ushort_t port, int n) :
        m_ios_pool(*factory<io_service_pool*>()(n)),
        m_acceptor(m_ios_pool.get(),
        ip::tcp::endpoint(ip::tcp::v4(), port)),
#if  __cplusplus >= 201103L
        m_vclient(NULL),
#endif
        m_ws_interface(*this, m_ios_pool)
{
    start_accept();
}

tcp_server::tcp_server(io_service_pool &ios, ushort port) :
        m_ios_pool(ios),
        m_acceptor(m_ios_pool.get(),
        ip::tcp::endpoint(ip::tcp::v4(), port)),
#if  __cplusplus >= 201103L
        m_vclient(NULL),
#endif
        m_ws_interface(*this, m_ios_pool)
{
    start_accept();
}

void tcp_server::start_accept()
{
    LOG_DEBUG("start accept");
    tcp_session_ptr session =
            factory<tcp_session_ptr>() (m_ios_pool.get(), m_ws_interface);

    m_acceptor.async_accept(session->socket(),
            bind(&tcp_server::handle_accept, this,
                    boost::asio::placeholders::error, session));
}

void tcp_server::handle_accept(const system::error_code &error,
                                                tcp_session_ptr session)
{
    start_accept();

    if(error)
    {
        session->close();
        return;
    }

    m_mutex.lock();
    m_vclient = session;
    m_mutex.unlock();
    session->io_service().dispatch(bind(&tcp_session::start, session));
}

void tcp_server::start()
{
    m_ios_pool.start();
    m_ws_interface.run();
}

void tcp_server::run()
{
    m_ios_pool.run();
}

void tcp_server::send_to_vclient(const uint32_t msg_type)
{
    m_mutex.lock();
#if  __cplusplus >= 201103L
    if(m_vclient) {
#endif
        m_vclient->message_out_handle(msg_type);
#if  __cplusplus >= 201103L
    }
#endif
    m_mutex.unlock();
}
