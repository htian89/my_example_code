/*
 * tcp_message.h
 *
 *  Created on: 2015年1月6日
 *      Author: root
 */

#ifndef TCP_MESSAGE_H_
#define TCP_MESSAGE_H_

#include "vclient_common.h"

typedef struct MsgHeader {
    uint32_t client_type;
    uint32_t msg_type;
} MsgHeader;

enum {
    CLIENT_TYPE_WEBSOCKET = 0,
    CLIENT_TYPE_VCLIENT,
    CLIENT_TYPE_FAP,

    CLIENT_TYPE_END
};

enum {
    VCLIENT_MSG_SET_ADDRESS = 0,
    VCLIENT_MSG_SET_SESSION_STATUS,
    VCLIENT_MSG_LOGOFF,
    VCLIENT_MSG_MULTICAST,
    VCLIENT_MSG_SET_SEAT_NUMBER,
    VCLIENT_MSG_SHOW_SEAT_NUMBER,
    VCLIENT_MSG_START_BROADCAST,
    VCLIENT_MSG_END_BROADCAST,
    VCLIENT_MSG_NOTES,

    VCLIENT_MSG_END
};

enum {
    VCLIENT_MSGC_SET_ADDRESS = 0,
    VCLIENT_MSGC_SET_SESSION_STATUS,
    VCLIENT_MSGC_LOGOFF,
    VCLIENT_MSGC_MULTICAST,
    VCLIENT_MSGC_SET_SEAT_NUMBER,
    VCLIENT_MSGC_SHOW_SEAT_NUMBER,
    VCLIENT_MSGC_START_BROADCAST,
    VCLIENT_MSGC_END_BROADCAST,
    VCLIENT_MSGC_NOTES,

    VCLENT_MSGC_END
};

typedef struct vClientMsgcSetAddress {
    uchar_t ip[IP_SIZE];
    uint16_t port;
} vClientMsgcSetAddress;

typedef struct vClientMsgcSetSessionStatus {
    uint32_t status;
    uchar_t ticket[MAX_SIZE];
} vClientMsgcSetSessionStatus;

typedef struct vClientMsgcSetSeatNumber {
    uchar_t seat_number[MAX_SIZE];
} vClientMsgcSetSeatNumber;

typedef struct vClientMsgShowSeatNumber {
    uchar_t seat_number[MAX_SIZE];
} vClientMsgShowSeatNumber;

typedef struct vClientMsgStartBroadCast {

} vClientMsgStartBroadCast;

typedef struct vClientMsgEndBroadCast {

} vClientMsgEndBroadCast;

typedef struct vClientMsgNotes{
    uchar_t send_notes[MAX_SIZE];
}vClientMsgNotes;

#endif /* TCP_MESSAGE_H_ */
