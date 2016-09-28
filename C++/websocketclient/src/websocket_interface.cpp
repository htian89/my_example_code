/*
 * websocket_interface.cpp
 *
 *  Created on: 2014年12月26日
 *      Author: root
 */

#include <cstdlib>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "websocket_interface.h"
#include "tcp_server.h"
#include "process_manager.hpp"

#include "common/utils.h"
#include "common/base64tools.hpp"
#include "common/vclient_common.h"

#if __cplusplus >= 201103L
using namespace web::websockets;
using namespace web::websockets::client;
#endif

using namespace std;
using namespace boost;

websocket_interface::websocket_interface(tcp_server &obj, io_service_pool &ios_pool):
        m_tcp_server(obj),
        m_ios_pool(ios_pool),
        ws_client(NULL),
        ws_client_connected(0),
        m_heart_token(0)
{
}

void websocket_interface::run()
{
    connect_to_vaccess();
    m_ios_pool.get().dispatch(boost::bind(&websocket_interface::heart_beat_thread, this));
    m_ios_pool.get().dispatch(boost::bind(&websocket_interface::recv_from_vaccess, this));
}

void websocket_interface::connect_to_vaccess()
{
    m_ws_client_mutex.lock();
    while(1)
    {
        if(ws_client)
        {
            delete ws_client;
            ws_client = NULL;
        }

        ws_client = new websocket_client;

#if __cplusplus >= 201103L
        VERIFY_THROW(m_uri = web::uri("ws://" + get_websocket_ip() + ":" + boost::lexical_cast<string>(get_websocket_port()) + "/ws"), {sleep(2);  continue;});
        LOG_DEBUG("connect to: " << m_uri.to_string());
        VERIFY_THROW(ws_client->connect(m_uri).wait(), {sleep(2); continue;});
#else
        std::string ip = get_websocket_ip();
        ushort_t port = get_websocket_port();
        LOG_DEBUG("ip = " << ip <<"; port " << port);
        VERIFY_THROW(ws_client->connect(ip, port), {sleep(2); continue;});
#endif
        VERIFY_THROW(global_data_init(), {sleep(2); continue;});
        break;
    }
    m_ws_client_mutex.unlock();

    message_out_handle("Hello");
}

void websocket_interface::disconnect_to_vaccess() {
    if(m_ws_client_mutex.try_lock())
    {
        VERIFY_THROW(ws_client->close());
        m_ws_client_mutex.unlock();
    }
}

void websocket_interface::reconnect_to_vaccess() {
    disconnect_to_vaccess();
}

void websocket_interface::recv_from_vaccess() {
    while(1)
    {
#if __cplusplus >= 201103L
        websocket_incoming_message ret_msg;
        VERIFY_THROW(ret_msg = ws_client->receive().get(), {connect_to_vaccess(); continue;});
        if(ret_msg.length() > 0)
        {
            std::string msg_in_base64;
            VERIFY_THROW(msg_in_base64 = ret_msg.extract_string().get(), continue);
#else
        const char *msg_in_base64;
        VERIFY_THROW(msg_in_base64 = ws_client->receive(), {connect_to_vaccess(); continue;});
        if(ws_client->received_size() > 0)
        {
#endif
            std::string msg_in;
            VERIFY_THROW(msg_in = base64tools::base64_decode(msg_in_base64), continue);
            LOG_DEBUG("msg in : " << msg_in);
            message_in_handle(msg_in);
        } else {
            LOG_DEBUG("recv timeout!");
        }
    }
}

void websocket_interface::send_to_vaccess(const std::string &msg_out_str) {
    std::string msg_out_base64 = base64tools::base64_encode(msg_out_str);
#if __cplusplus >= 201103L
    websocket_outgoing_message msg_out;
    msg_out.set_utf8_message(msg_out_base64);
#endif
    if(m_ws_client_mutex.try_lock())
    {
        LOG_DEBUG(msg_out_str);
#if  __cplusplus >= 201103L
        VERIFY_THROW(ws_client->send(msg_out).wait());
#else
        VERIFY_THROW(ws_client->send(msg_out_base64));
#endif
        m_ws_client_mutex.unlock();
    }
}

void websocket_interface::heart_beat_thread() {
    LOG_DEBUG("thread of heart beat is running");
    while(1)
    {
        sleep(10);
        m_mutex.lock();
        if(m_heart_token >= 3)
        {
            m_heart_token = 0;
            m_mutex.unlock();
            if(get_session_status())
                m_tcp_server.send_to_vclient(VCLIENT_MSG_LOGOFF);
            LOG_ERR("heartbeat was timeout, token = " << m_heart_token);
            disconnect_to_vaccess();
        } else {
            if(!m_ws_client_mutex.try_lock()) {
                m_mutex.unlock();
                continue;
            }
            m_ws_client_mutex.unlock();
            m_heart_token++;
            m_mutex.unlock();
            message_out_handle("HeartBeat");
        }
    }
}

void websocket_interface::message_in_handle(const std::string &msg_in)
{
    ptree obj;
    std::istringstream msg_in_stream(msg_in.c_str());
    VERIFY_THROW(read_json(msg_in_stream, obj), return);

    std::string action;
    VERIFY_THROW(action = obj.get<string>("Action"), return);

    if(action == "GetTerminalInfo") {
        GetTerminalInfo(obj);
    } else if(action == "NoticeServerType") {
        NoticeServerType(obj);
    } else if(action == "PowerOff") {
        PowerOff(obj);
    } else if(action == "Reboot") {
        Reboot(obj);
    } else if(action == "Notes") {
        Notes(obj);
    } else if(action == "Logoff") {
        Logoff(obj);
    } else if(action == "UpgradeSystem") {
        UpdateSystem(obj);
    } else if(action == "StartBroadcastScreen") {
        StartBroadcastScreen(obj);
    } else if(action == "EndBroadcastScreen") {
        EndBroadcastScreen(obj);
    } else if(action == "HeartBeat") {
        HeartBeat(obj);
    } else if(action == "SetSeatNumber") {
        SetSeatNumber(obj);
    } else if(action == "ChangeOccupyDesktop") {
        ChangeOccupyDesktop(obj);
    } else if(action == "ClearOccupyDesktop") {
        ClearOccupyDesktop(obj);
    } else if(action == "ShowSeatNumber") {
        ShowSeatNumber(obj);
    } else {
        LOG_DEBUG("unsupport action");
    }
}

void websocket_interface::GetTerminalInfo(ptree &msg_in)
{
    bool upgrade_mode;
    VERIFY_THROW(upgrade_mode = msg_in.get<int>("UpgradeMode"), upgrade_mode = false);
    set_upgrade_mode(upgrade_mode);

    ptree obj;
    std::string network_name = get_network_name();
    obj.put("Action","SetTerminalInfo");
    obj.put("Uuid",get_uuid());
    obj.put("Version",getProductVersion());
    obj.put("Product",getProductName());
    obj.put("HostName",getHostName());
    obj.put("Ip",getIpAddress(network_name));
    obj.put("Netmask",getNetmask(network_name));
    obj.put("Mac",getMacAddress(network_name));
    obj.put("Os",getOsVersion());
    obj.put("Cpu",getCpuInfo());
    obj.put("Memory",getMemory());
    obj.put("GraphicsCard",getGraphicsCard());
    obj.put("SoundCard",getSoundCard());

    std::stringstream msg_out;
    write_json(msg_out, obj, false);
    send_to_vaccess(msg_out.str());
    return;
}

void websocket_interface::NoticeServerType(ptree &msg_in)
{
    std::string server_type;
    VERIFY_THROW(server_type = msg_in.get<string>("Value"), return);
    set_server_type(server_type);
}

void websocket_interface::PowerOff(ptree &msg_in)
{
    std::system("poweroff");
}

void websocket_interface::Reboot(ptree &msg_in)
{
    std::system("reboot");
}

void websocket_interface::Notes(ptree &msg_in)
{
    std::string notes;
    VERIFY_THROW(notes = msg_in.get<string>("Msg"), return);
    set_notes(notes);
    m_tcp_server.send_to_vclient(VCLIENT_MSG_NOTES);
}

void websocket_interface::Logoff(ptree &msg_in)
{
    m_tcp_server.send_to_vclient(VCLIENT_MSG_LOGOFF);
}

void websocket_interface::UpdateSystem(ptree &msg_in)
{
    std::string upgrade_source_ip;
    std::string cmd;
    VERIFY_THROW(upgrade_source_ip = msg_in.get<string>("SourceIp"), upgrade_source_ip.clear());
    LOG_DEBUG("not support auto update system : " << upgrade_source_ip);
    if( upgrade_source_ip.size() > 0) {
        cmd = UPDATE_FILE + " --autoupdate SourceIp=" + upgrade_source_ip;
    } else {
        if(get_upgrade_mode() == 1) {
            cmd = UPDATE_FILE + " --autoupdate --automode";
        }else{
            cmd = UPDATE_FILE + " --autoupdate";
        }
    }
    LOG_DEBUG(cmd);
    process_manager::create_process(cmd);
}

void websocket_interface::ChangeOccupyDesktop(ptree &msg_in)
{
    std::string user_name;
    VERIFY_THROW(user_name = msg_in.get<string>("Username"), return);
    writeUserName(user_name);
    m_tcp_server.send_to_vclient(VCLIENT_MSG_LOGOFF);
}

void websocket_interface::ClearOccupyDesktop(ptree &msg_in)
{
    writeUserName("");
    m_tcp_server.send_to_vclient(VCLIENT_MSG_LOGOFF);
}

void websocket_interface::SetSeatNumber(ptree &msg_in)
{
    m_tcp_server.send_to_vclient(VCLIENT_MSG_SET_SEAT_NUMBER);
}

void websocket_interface::ShowSeatNumber(ptree &msg_in)
{
    std::string seat_number;
    VERIFY_THROW(seat_number = msg_in.get<string>("SeatNumber"), return);
    set_seat_number(seat_number);
    m_tcp_server.send_to_vclient(VCLIENT_MSG_SHOW_SEAT_NUMBER);
}

void websocket_interface::StartBroadcastScreen(ptree &msg_in)
{
    if(get_process_id())
        process_manager::stop_process(get_process_id());

    std::string ip;
    ushort_t port;
    VERIFY_THROW(ip = msg_in.get<string>("Ip"), return);
    VERIFY_THROW(port = msg_in.get<int>("Port"), return);
    int vlc_pid = process_manager::create_process(ip, port);
    LOG_DEBUG("vlc pid " << vlc_pid);
    set_process_id(vlc_pid);
    m_tcp_server.send_to_vclient(VCLIENT_MSG_START_BROADCAST);
}

void websocket_interface::EndBroadcastScreen(ptree &msg_in)
{
    LOG_DEBUG("get vlc pid " << get_process_id());
    if(get_process_id())
        process_manager::stop_process(get_process_id());
    m_tcp_server.send_to_vclient(VCLIENT_MSG_END_BROADCAST);
}

void websocket_interface::HeartBeat(ptree &msg_in)
{
    uint32_t error_code;
    VERIFY_THROW(error_code = msg_in.get<int>("ErrorCode"), return);
    switch(error_code)
    {
    case 0:
        m_mutex.lock();
        m_heart_token = 0;
        m_mutex.unlock();
        break;
    case -101:
    case -102:
    case -103:
    default:
        LOG_ERR("recv heart beat errcode: " << error_code);
        m_tcp_server.send_to_vclient(VCLIENT_MSG_LOGOFF);
        break;
    }
}

void websocket_interface::message_out_handle(const std::string &action)
{
    ptree obj;
    obj.put("Action", action);
    if (action == "Hello") {
        obj.put("Uuid", get_uuid());
    } else if (action == "HeartBeat") {
        obj.put("Status", int(get_session_status()));
        if(get_session_status())
            obj.put("Ticket", get_logon_ticket());
    } else if (action == "CommitSeatNumber") {
        obj.put("SeatNumber", get_seat_number());
    } else {
        LOG_DEBUG("unsupport action");
    }
    std::stringstream msg_out;
    write_json(msg_out, obj, false);
    send_to_vaccess(msg_out.str());
    return;
}
