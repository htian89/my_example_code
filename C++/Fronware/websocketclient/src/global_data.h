/*
 * global_data.h
 *
 *  Created on: 2014年12月26日
 *      Author: root
 */

#ifndef GLOBAL_DATA_H_
#define GLOBAL_DATA_H_

#include <string>
#include <boost/thread/mutex.hpp>

#include "config_parse.h"

#include "common/terminal_info.h"
#include "common/tcp_message.h"

class global_data : public config_parse, public terminal_info {
public:
    global_data();
    void global_data_init();
    std::string get_network_name();
    std::string get_uuid();
    void set_websocket_port(const ushort_t &port);
    ushort get_websocket_port();
    void set_websocket_ip(const std::string &ip);
    std::string get_websocket_ip();
    void set_logon_ticket(const std::string &ticket, const bool &status);
    bool get_session_status();
    std::string get_logon_ticket();
    void set_server_type(const std::string &server_type);
    std::string get_server_type();
    void set_seat_number(const std::string &seat_number);
    std::string get_seat_number();
    void set_upgrade_mode(const bool upgrade_mode);
    bool get_upgrade_mode();
    void clear_process_id(const int pid);
    void set_process_id(const int pid);
    int get_process_id();
    void set_notes(const std::string &notes);
    std::string get_notes();

private:
    boost::mutex m_mutex;
    std::string m_network_name;
    std::string m_uuid;
    std::string m_server_type;
    std::string m_server_ip;
    ushort_t  m_server_port;
    bool m_session_status;
    std::string m_logon_ticket;
    std::string m_seat_number;
    bool m_upgrade_mode;
    int m_vlc_pid;
    std::string m_notes;
};

#endif /* GLOBAL_DATA_H_ */
