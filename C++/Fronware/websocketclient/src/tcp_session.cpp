/*
 * tcp_session.cpp
 *
 *  Created on: 2015年1月6日
 *      Author: root
 */

#include "tcp_session.h"

#include <iostream>
#include <string>
#include <cstring>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "common/utils.h"
#include "common/tcp_message.h"


using namespace boost;
using namespace boost::asio;

tcp_session::tcp_session(ios_type &ios, websocket_interface &ws_i):
        m_socket(ios),
        m_ws_interface(ws_i)
        { }

tcp_session::socket_type &tcp_session::socket()
{   return m_socket;}

tcp_session::ios_type &tcp_session::io_service()
{   return m_socket.get_io_service(); }

tcp_session::buffer_type &tcp_session::read_buf()
{   return m_read_buf; }

tcp_session::buffer_type &tcp_session::write_buf()
{   return m_write_buf; }

void tcp_session::start()
{
    read();
}

void tcp_session::close()
{
    boost::system::error_code   ignored_ec;
    m_socket.shutdown(ip::tcp::socket::shutdown_both, ignored_ec);
    m_socket.close(ignored_ec);
}

void tcp_session::read()
{
    m_socket.async_read_some(
            m_read_buf.prepare(),
            bind(&tcp_session::handle_read, shared_from_this(),
                    placeholders::error, placeholders::bytes_transferred)
            );
}

void tcp_session::handle_read(const system::error_code &error,
        size_t bytes_transferred)
{
    if(error)
    {
        close();
        return;
    }

    m_read_buf.retrieve(bytes_transferred);

    const char *data = read_buf().peek();
    LOG_DEBUG("data size: " << bytes_transferred);
    MsgHeader *header = (MsgHeader *)data;
    LOG_DEBUG("client type = " << header->client_type);
    LOG_DEBUG("msg type = " << header->msg_type);
    const char *msg_data = data + sizeof(MsgHeader);
    switch(header->msg_type) {
    case VCLIENT_MSGC_SET_ADDRESS: {
        vClientMsgcSetAddress *msg_in = (vClientMsgcSetAddress *)(msg_data);
        LOG_DEBUG("ip = " << msg_in->ip);
        LOG_DEBUG("port = " << msg_in->port);
        if(m_ws_interface.get_websocket_ip() != std::string((const char *)(msg_in->ip)))
        {
            m_ws_interface.set_websocket_ip(std::string((const char *)(msg_in->ip)));
            m_ws_interface.reconnect_to_vaccess();
        }
        break;
    }
    case VCLIENT_MSGC_SET_SESSION_STATUS:{
        vClientMsgcSetSessionStatus *msg_in = (vClientMsgcSetSessionStatus *)(msg_data);
        LOG_DEBUG("status = " << msg_in->status);
        LOG_DEBUG("ticket = " << msg_in->ticket);
        m_ws_interface.set_logon_ticket(std::string((const char *)(msg_in->ticket)), msg_in->status);
        break;
    }
    case VCLIENT_MSGC_SET_SEAT_NUMBER: {
        vClientMsgcSetSeatNumber *msg_in = (vClientMsgcSetSeatNumber *)(msg_data);
        LOG_DEBUG("seat number = " << (const char *)(msg_in->seat_number));
        m_ws_interface.set_seat_number(std::string((const char *)(msg_in->seat_number)));
        m_ws_interface.writeSeatNumber(std::string((const char *)(msg_in->seat_number)));
        m_ws_interface.message_out_handle("CommitSeatNumber");
        break;
    }
    default:
        LOG_ERR("unsupport msg type");
        break;
    }

    m_read_buf.consume(bytes_transferred);
    read();
}

void tcp_session::write(const void *data, size_t len)
{
    m_write_buf.append(data, len);

    write();
}

void tcp_session::write()
{
    m_socket.async_write_some(
            m_write_buf.data(),
            bind(&tcp_session::handle_write, shared_from_this(),
                    placeholders::error, placeholders::bytes_transferred)
            );
}

void tcp_session::handle_write(const system::error_code &error,
        size_t bytes_transferred)
{
    if(error)
    {
        close();
        return;
    }

    m_write_buf.consume(bytes_transferred);
}

void tcp_session::message_out_handle(const uint32_t msg_type)
{
    int32_t header_size = sizeof(MsgHeader);
    switch(msg_type)
    {
    case VCLIENT_MSG_LOGOFF:
    {
        MsgHeader header = {CLIENT_TYPE_WEBSOCKET, msg_type};
        write(&header, sizeof(header));
        break;
    }
    case VCLIENT_MSG_SET_SEAT_NUMBER:
    {
        MsgHeader header = {CLIENT_TYPE_WEBSOCKET, msg_type};
        write(&header, sizeof(header));
        break;
    }
    case VCLIENT_MSG_SHOW_SEAT_NUMBER:
    {
        int32_t msg_size = header_size + sizeof(vClientMsgShowSeatNumber);
        uchar_t *msg_out = new uchar_t[msg_size]();

        MsgHeader *header = (MsgHeader *)msg_out;
        header->client_type = CLIENT_TYPE_WEBSOCKET;
        header->msg_type = msg_type;

        vClientMsgShowSeatNumber *data = (vClientMsgShowSeatNumber *)(msg_out + header_size);
        std::strcpy((char *)(data->seat_number), m_ws_interface.get_seat_number().c_str());

        write(msg_out, msg_size);
        delete [] msg_out;
        break;
    }
    case VCLIENT_MSG_START_BROADCAST:
    {
        MsgHeader header = {CLIENT_TYPE_WEBSOCKET, msg_type};
        write(&header, sizeof(header));
        break;
    }
    case VCLIENT_MSG_END_BROADCAST:
    {
        MsgHeader header = {CLIENT_TYPE_WEBSOCKET, msg_type};
        write(&header, sizeof(header));
        break;
    }
    case VCLIENT_MSG_NOTES:
    {
        int32_t msg_size = header_size + sizeof(vClientMsgNotes);
        uchar_t *msg_out = new uchar_t[msg_size]();

        MsgHeader *header = (MsgHeader *)msg_out;
        header->client_type = CLIENT_TYPE_WEBSOCKET;
        header->msg_type = msg_type;

        vClientMsgNotes *data = (vClientMsgNotes *)(msg_out + header_size);
        std::strcpy((char *)(data->send_notes), m_ws_interface.get_notes().c_str());

        write(msg_out, msg_size);
        delete [] msg_out;
        break;
    }
    default:
        LOG_ERR("unsupport message out type : " << msg_type);
        break;
    }
}
