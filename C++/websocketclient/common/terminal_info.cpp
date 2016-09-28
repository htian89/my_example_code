/*
 * terminal_info.cpp
 *
 *  Created on: 2014年12月24日
 *      Author: root
 */


#include "terminal_info.h"

#include <stdio.h>
#include <cstring>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "common/utils.h"
#include "common/vclient_common.h"

using namespace std;
using namespace boost::uuids;

std::string terminal_info::getProductVersion()
{
    ifstream fin(VERSION_FILE.c_str());
    if(fin.is_open())
    {
        std::string ret;
        fin >> ret;
        int len = ret.size();
        if(len) {
            if (ret.at(len - 1) == '\n' || ret.at(len - 1) == '\r')
                ret.replace(len - 1, 1, "\0");
        }
        return ret;
    } else {
        throw vclient_exception("read file: " + VERSION_FILE + " failed!");
    }
}

std::string terminal_info::getProductName()
{
    return PRODUCT_NAME;
}

std::string terminal_info::getProductUuid()
{
    ifstream fin(UUID_FILE.c_str());
    if(fin.is_open())
    {
        std::string ret;
        fin >> ret;
        int len = ret.size();
        if(len) {
            if (ret.at(len - 1) == '\n' || ret.at(len - 1) == '\r')
                ret.replace(len - 1, 1, "\0");
        }
        return ret;
    } else {
        ofstream fout(UUID_FILE.c_str());
        if(fout.is_open())
        {
            boost::uuids::uuid _uuid = random_generator()();
            std::stringstream _uuid_ss;
            _uuid_ss << _uuid;
            std::string uuid(_uuid_ss.str());
            fout.write(uuid.c_str(), uuid.size());
            fout.close();
            return uuid;
        } else {
            throw vclient_exception("create file: " + UUID_FILE + " failed!");
        }
    }
}

std::string terminal_info::readSeatNumber()
{
    ifstream fin(SEAT_NUMBER_FILE.c_str());
    if(fin.is_open())
    {
        std::string ret;
        fin >> ret;
        int len = ret.size();
        if(len) {
            if (ret.at(len - 1) == '\n' || ret.at(len - 1) == '\r')
                ret.replace(len - 1, 1, "\0");
        }
        return ret;
    } else {
        throw vclient_exception("read file: " + SEAT_NUMBER_FILE + " failed!");
    }
}

void terminal_info::writeMacAddress(std::string &mac)
{
    ofstream fout(MAC_ADDRESS_FILE.c_str());
    if(fout.is_open())
    {
        fout.write(mac.c_str(), mac.size());
        fout.close();
    } else {
        throw vclient_exception("create file: " + MAC_ADDRESS_FILE + " failed!");
    }
}

std::string terminal_info::readMacAddress()
{
    ifstream fin(MAC_ADDRESS_FILE.c_str());
    std::string ret;
    if(fin.is_open())
    {
        fin >> ret;
        int len = ret.size();
        if(len) {
            if (ret.at(len - 1) == '\n' || ret.at(len - 1) == '\r')
                ret.replace(len - 1, 1, "\0");
        }
    }
    return ret;
}

void terminal_info::writeSeatNumber(const std::string &seat_number)
{
    ofstream fout(SEAT_NUMBER_FILE.c_str());
    if(fout.is_open())
    {
        fout.write(seat_number.c_str(), seat_number.size());
        fout.close();
    } else {
        throw vclient_exception("create file: " + SEAT_NUMBER_FILE + " failed!");
    }
}

void terminal_info::writeUserName(const std::string &user_name)
{
    ofstream fout(USER_NAME_FILE.c_str());
    if(fout.is_open())
    {
        fout.write(user_name.c_str(), user_name.size());
        fout.close();
    } else {
        throw vclient_exception("create file: " + USER_NAME_FILE + " failed!");
    }
}

std::string terminal_info::getCpuInfo()
{
#ifdef WEBSOCKET_CLIENT_V200
    return excuteSystemCmdInfo(std::string("cat /proc/cpuinfo | grep Processor | cut -f2 | uniq | awk -F: '{print$2}'"));
#else
    return excuteSystemCmdInfo(std::string("cat /proc/cpuinfo | grep name | cut -f2 | uniq | awk -F: '{print$2}'"));
#endif
}

std::string terminal_info::getHostName()
{
    return excuteSystemCmdInfo(std::string("hostname"));
}

std::string terminal_info::getNetworkName()
{
#ifdef WEBSOCKET_CLIENT_V200
    return "eth0";
#else
    return excuteSystemCmdInfo(std::string("netstat -r | grep default | awk -F' ' '{print $8}'"));
#endif
}

std::string terminal_info::getIpAddress(std::string &name)
{
#ifdef WEBSOCKET_CLIENT_V200
    std::string cmd = "ifconfig " + name + " |  grep inet | grep -v inet6 | awk '{print $2}' | awk -F':' '{print $2}'";
#else
    std::string cmd = "ifconfig " + name + " |  grep inet | grep -v inet6 | awk '{print $2}'";
#endif
    std::string retValue = excuteSystemCmdInfo(cmd);
    return getFirstLine(retValue);
}

std::string terminal_info::getMacAddress(std::string &name)
{
#ifdef WEBSOCKET_CLIENT_V200
    std::string cmd = "ifconfig " + name + " |  grep HWaddr | awk -F' ' '{print $5}'";
#else
    std::string cmd = "ifconfig " + name + " |  grep ether | awk -F' ' '{print $2}'";
#endif
    std::string retValue = excuteSystemCmdInfo(cmd);
    return getFirstLine(retValue);
}

std::string terminal_info::getNetmask(std::string &name)
{
#ifdef WEBSOCKET_CLIENT_V200
    std::string cmd = "ifconfig " + name + " |  grep inet | grep -v inet6 | awk '{print $4}' | awk -F':' '{print $2}'";
#else
    std::string cmd = "ifconfig " + name + " |  grep inet | grep inet | grep -v inet6 | awk '{print $4}'";
#endif
    std::string retValue = excuteSystemCmdInfo(cmd);
    return getFirstLine(retValue);
}

std::string terminal_info::getMemory()
{
#ifdef WEBSOCKET_CLIENT_V200
    std::string retValue = "1024";
#else
    std::string retValue = excuteSystemCmdInfo(std::string("free -m | grep Mem | awk '{print $2}'"));
#endif
    retValue += "MB";
    return retValue;
}

std::string terminal_info::getGraphicsCard()
{
#ifdef WEBSOCKET_CLIENT_V200
    return "";
#else
    return excuteSystemCmdInfo(std::string("lspci |grep VGA | awk -F: '{print $3}'"));
#endif
}

std::string terminal_info::getSoundCard()
{
#ifdef WEBSOCKET_CLIENT_V200
    return "";
#else
    return excuteSystemCmdInfo(std::string("lspci |grep Audio | awk -F: '{print $3}'"));
#endif
}

std::string terminal_info::getOsVersion()
{
    return excuteSystemCmdInfo(std::string("cat /proc/version | awk '{print $1 $3}'"));
}

bool terminal_info::getVlcStatus()
{
    std::string retValue = excuteSystemCmdInfo(std::string("ps ax | grep 'vlc' | grep 'no-mouse-events' | grep -v 'grep'| wc -l"));
    return !retValue.empty();
}

std::string terminal_info::excuteSystemCmdInfo(const std::string cmd)
{
    if(cmd.size() == 0)
        throw vclient_exception("illegal commad!");

    std::string tRet;
    FILE *fpRead;
    int len;

    fpRead = popen(cmd.c_str(), "r");

    char str[4096] = {0};
    while (fgets(str, 4096 - 1, fpRead) != NULL)
    {
        tRet = tRet + std::string(str);
        std::memset(str, 0, sizeof(str));
    }

    len = tRet.size();
    if(len) {
        if (tRet.at(len-1) == '\n' || tRet.at(len-1) == '\r')
            tRet.replace(len-1, 1, "\0");
    }

    if(fpRead != NULL)
        pclose(fpRead);

    return tRet;
}

std::string terminal_info::getFirstLine(std::string &value)
{
    int len = value.size();
    if(len == 0)
        return value;

    int pos = value.find_first_of("\n");
    if(pos != std::string::npos) {
        value.replace(pos, 1, "\0");
        value.erase(pos , len - pos);
    }
    return value;
}
