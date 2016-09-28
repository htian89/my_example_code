/*
 * terminalinfo_test.cpp
 *
 *  Created on: 2014年12月24日
 *      Author: root
 */

#include "common/utils.h"
#include "common/terminal_info.h"


int main()
{
    terminal_info info;
    std::string network_name = info.getNetworkName();
    LOG_DEBUG(info.getProductUuid());
    LOG_DEBUG(info.getProductName());
    LOG_DEBUG(info.getOsVersion());
    LOG_DEBUG(info.getProductVersion());
    LOG_DEBUG(info.getHostName());
    LOG_DEBUG(info.getIpAddress(network_name));
    LOG_DEBUG(info.getNetmask(network_name));
    LOG_DEBUG(info.getMacAddress(network_name));
    LOG_DEBUG(info.getCpuInfo());
    LOG_DEBUG(info.getMemory());
    LOG_DEBUG(info.getGraphicsCard());
    VERIFY_THROW(LOG_DEBUG(info.getSoundCard()));
}
