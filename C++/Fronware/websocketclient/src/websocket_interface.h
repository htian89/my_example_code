/*
 * websocket_interface.h
 *
 *  Created on: 2014年12月26日
 *      Author: root
 */

#ifndef WEBSOCKET_INTERFACE_H_
#define WEBSOCKET_INTERFACE_H_

#include <string>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#if  __cplusplus >= 201103L
#include "cpprest/asyncrt_utils.h"
#include "cpprest/rawptrstream.h"
#include "cpprest/containerstream.h"
#include "cpprest/producerconsumerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/basic_types.h"
#include "cpprest/ws_client.h"
#include "cpprest/ws_msg.h"
using namespace web::websockets::client;
#else
#include "websocket_client.h"
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
using namespace boost::property_tree;

#include "global_data.h"
#include "common/io_service_pool.hpp"

class tcp_server;

class websocket_interface : public global_data{
public:
    websocket_interface(tcp_server &obj, io_service_pool &ios_pool);
    void run();
    void send_to_vaccess(const std::string &msg_out_str);
    void disconnect_to_vaccess();
    void reconnect_to_vaccess();
    void message_out_handle(const std::string &action);

private:
    void connect_to_vaccess();
    void recv_from_vaccess();
    void heart_beat_thread();

private: //message in
    void message_in_handle(const std::string &msg_in);
    void GetTerminalInfo(ptree &msg_in);
    void NoticeServerType(ptree &msg_in);
    void PowerOff(ptree &msg_in);
    void Reboot(ptree &msg_in);
    void Notes(ptree &msg_in);
    void Logoff(ptree &msg_in);
    void UpdateSystem(ptree &msg_in);
    void ChangeOccupyDesktop(ptree &msg_in);
    void ClearOccupyDesktop(ptree &msg_in);
    void SetSeatNumber(ptree &msg_in);
    void ShowSeatNumber(ptree &msg_in);
    void StartBroadcastScreen(ptree &msg_in);
    void EndBroadcastScreen(ptree &msg_in);
    void HeartBeat(ptree &msg_in);

private:
    tcp_server &m_tcp_server;
    boost::mutex m_ws_client_mutex;
    websocket_client *ws_client;
#if  __cplusplus >= 201103L
    web::uri m_uri;
#endif
    boost::mutex m_mutex;
    int m_heart_token;
    bool ws_client_connected;
    io_service_pool &m_ios_pool;
};

#endif /* WEBSOCKET_INTERFACE_H_ */
