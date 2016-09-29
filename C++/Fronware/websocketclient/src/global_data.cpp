/*
 * global_data.cpp
 *
 *  Created on: 2014年12月26日
 *      Author: root
 */

#include <cstring>

#include "global_data.h"
#include "common/utils.h"

#define WEBSOCKET_SERVER_PORT 9000

global_data::global_data() :
m_server_port(WEBSOCKET_SERVER_PORT),
m_session_status(0),
m_vlc_pid(0)
{
    VERIFY_THROW(m_seat_number = readSeatNumber());
    VERIFY_THROW(m_server_ip = getNetworkInfo());
    LOG_DEBUG(m_server_ip);
    std::memset(&m_session_status, 0, sizeof(m_session_status));
}

void global_data::global_data_init()
{
    m_network_name = getNetworkName();
    if(m_network_name.empty())
    {
        throw vclient_exception("get network card name failed!");
    }

    std::string mac = getMacAddress(m_network_name);
    if(mac.empty())
    {
        throw vclient_exception("get mac address failed!");
    }

    std::string old_mac = readMacAddress();
    if(mac != old_mac)
    {
#ifndef WEBSOCKET_CLIENT_V200
        if(old_mac.empty())
        {
            LOG_DEBUG("system first start");
        } else {
            LOG_DEBUG("install by clonezilla");
        }
        std::string cmd = "rm -f " +  UUID_FILE + " " + MAC_ADDRESS_FILE + " " + SEAT_NUMBER_FILE;
        system(cmd.c_str());
#endif
        writeMacAddress(mac);
    }

    m_mutex.lock();
    m_uuid = getProductUuid();
    m_mutex.unlock();
}

std::string global_data::get_network_name()
{
    return m_network_name;
}

std::string global_data::get_uuid()
{
    std::string uuid;
    m_mutex.lock();
    uuid = m_uuid;
    m_mutex.unlock();
    return uuid;
}

void global_data::set_websocket_port(const ushort_t &port)
{
    m_mutex.lock();
    m_server_port = port;
    m_mutex.unlock();
    LOG_DEBUG("set websocket port: " << port);
}

ushort_t global_data::get_websocket_port()
{
    ushort_t port;
    m_mutex.lock();
    port = m_server_port;
    m_mutex.unlock();
    return port;
}

void global_data::set_websocket_ip(const std::string &ip)
{
    m_mutex.lock();
    m_server_ip = ip;
    m_mutex.unlock();
    LOG_DEBUG("set websocket ip: " << ip);
}

std::string global_data::get_websocket_ip()
{
    std::string ip;
    m_mutex.lock();
    ip = m_server_ip;
    m_mutex.unlock();
    return ip;
}

void global_data::set_logon_ticket(const std::string &ticket, const bool &status)
{
    m_mutex.lock();
    m_session_status = status;
    if(status)
        m_logon_ticket = ticket;
    else
        m_logon_ticket.clear();
    m_mutex.unlock();
}

bool global_data::get_session_status()
{
    bool status;
    m_mutex.lock();
    status = m_session_status;
    m_mutex.unlock();
    return status;
}

std::string global_data::get_logon_ticket()
{
    std::string ticket;
    m_mutex.lock();
    ticket = m_logon_ticket;
    m_mutex.unlock();
    return ticket;
}

void global_data::set_server_type(const std::string &server_type)
{
    m_mutex.lock();
    m_server_type = server_type;
    m_mutex.unlock();
}

std::string global_data::get_server_type()
{
    std::string server_type;
    m_mutex.lock();
    server_type = m_server_type;
    m_mutex.unlock();
    return server_type;
}

void global_data::set_seat_number(const std::string &seat_number)
{
    m_mutex.lock();
    m_seat_number = seat_number;
    m_mutex.unlock();
}

std::string global_data::get_seat_number()
{
    std::string seat_number;
    m_mutex.lock();
    seat_number = m_seat_number;
    m_mutex.unlock();
    return seat_number;
}

void global_data::set_upgrade_mode(const bool upgrade_mode)
{
    m_mutex.lock();
    m_upgrade_mode = upgrade_mode;
    m_mutex.unlock();
}

bool global_data::get_upgrade_mode()
{
    bool upgrade_mode;
    m_mutex.lock();
    upgrade_mode = m_upgrade_mode;
    m_mutex.unlock();
    return upgrade_mode;
}

void global_data::clear_process_id(const int pid)
{
    if(m_vlc_pid == pid)
        m_vlc_pid = 0;
}

void global_data::set_process_id(const int pid)
{
    m_mutex.lock();
    m_vlc_pid = pid;
    m_mutex.unlock();
}

int global_data::get_process_id()
{
    int pid;
    m_mutex.lock();
    pid = m_vlc_pid;
    m_mutex.unlock();
    return pid;
}

void global_data::set_notes(const std::string &notes)
{
    m_mutex.lock();
    m_notes = notes;
    m_mutex.unlock();
}

std::string global_data::get_notes()
{
    std::string notes;
    m_mutex.lock();
    notes = m_notes;
    m_mutex.unlock();
    return notes;
}
