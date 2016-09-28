/*
 * vclient_common.h
 *
 *  Created on: 2014年12月23日
 *      Author: root
 */

#ifndef VCLIENT_COMMON_H_
#define VCLIENT_COMMON_H_
#include <string>

typedef uint8_t byte_t;
typedef uint8_t uchar_t;
typedef uint16_t word_t;
typedef uint32_t dword_t;
typedef unsigned short  ushort_t;
typedef unsigned int        uint_t;
typedef unsigned long       ulong_t;

const ushort_t IPC_SERVER_PORT = 6060;
const int MAX_SIZE = 512;
const int UUID_SIZE = 36;
const int IP_SIZE = 16;
const int MAC_SIZE = 20;

const std::string PRODUCT_NAME = "Fronview3000";
const std::string USER_PATH = std::string(getenv("HOME")) + std::string("/.vclient/");
const std::string UUID_FILE = USER_PATH + std::string("uuid");
const std::string NETWORK_FILE = USER_PATH + std::string("config.xml");
const std::string SEAT_NUMBER_FILE = USER_PATH + std::string("seat_number");
const std::string USER_NAME_FILE = USER_PATH + std::string("user_name");
const std::string MAC_ADDRESS_FILE = USER_PATH + std::string("mac_address");
const std::string DATA_PATH = std::string("/opt/vclient/");
const std::string VERSION_FILE = DATA_PATH + std::string("vclient_version");
const std::string UPDATE_FILE = DATA_PATH + std::string("update_module");
const std::string DEBUG_CONFIG_FILE = DATA_PATH + std::string("log.conf");

#endif /* VCLIENT_COMMON_H_ */
