/*
 * ipc_client.cpp
 *
 *  Created on: 2014年12月23日
 *      Author: root
 */

#include <boost/asio.hpp>

#include "common/utils.h"
#include "common/tcp_message.h"

using namespace boost;
using namespace boost::asio;

int main()
{
    LOG_DEBUG("client start");

    io_service ios;
    ip::tcp::socket sock(ios);

    ip::tcp::endpoint ep(
            ip::address::from_string("127.0.0.1"), IPC_SERVER_PORT);
    sock.connect(ep);

//    MsgHeader header = {CLIENT_TYPE_VCLIENT, VCLIENT_MSGC_SET_ADDRESS};
//    vClientMsgcSetAddress msg_out = { "192.168.22.2", 9000};
    MsgHeader header = {CLIENT_TYPE_VCLIENT, VCLIENT_MSGC_SET_SESSION_STATUS};
    vClientMsgcSetSessionStatus msg_out = {1, "testticket"};
    sock.write_some(buffer(&header, sizeof(header)));
    sock.write_some(buffer(&msg_out, sizeof(msg_out)));


//    std::vector<char> v(100, 0);
//    size_t n = sock.read_some(buffer(v));
//    LOG_DEBUG(&v[0]);
}
