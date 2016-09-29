/*
 * terminal_info.h
 *
 *  Created on: 2014年12月24日
 *      Author: root
 */

#ifndef TERMINAL_INFO_H_
#define TERMINAL_INFO_H_

#include <string>

class terminal_info
{
public:
    std::string getProductVersion();
    std::string getProductName();
    std::string getProductUuid();
    void writeMacAddress(std::string &mac);
    std::string readMacAddress();
    std::string readSeatNumber();
    void writeSeatNumber(const std::string &seat_number);
    void writeUserName(const std::string &user_name);
    std::string getHostName();
    std::string getNetworkName();
    std::string getIpAddress(std::string &name);
    std::string getNetmask(std::string &name);
    std::string getMacAddress(std::string &name);
    std::string getOsVersion();
    std::string getCpuInfo();
    std::string getMemory();
    std::string getGraphicsCard();
    std::string getSoundCard();
    bool getVlcStatus();
private:
    std::string excuteSystemCmdInfo(const std::string cmd);
    std::string getFirstLine(std::string &value);
};

#endif /* TERMINAL_INFO_H_ */
