/*
 * config_parse.cpp
 *
 *  Created on: 2014年12月24日
 *      Author: root
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "config_parse.h"

#include "common/utils.h"
#include "common/vclient_common.h"

using namespace boost::property_tree;

std::string config_parse::getNetworkInfo()
{
    ptree doc;
    read_xml(NETWORK_FILE, doc);
    std::string ip = doc.get<std::string>("VClientSettings.NetworkSettings.presentServer");
    return ip;
}
