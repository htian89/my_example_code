#ifndef __FRAME_CONSTANDT_H__
#define __FRAME_CONSTANDT_H__

//Note: the following 
#define HEADER_MAX_LEN 14
#define BASIC_HEADER_LEN  2

typedef struct _FRAME_Header_s
{
	
	 U8 opcode:4;
	 U8 RSV3:1;
 	 U8 RSV2:1;
 	 U8 RSV1:1;
	 U8 FIN:1;

	 U8 MASK:1;
	 U8 payload_len:7;

	 U16 extended_payload_len_16;
	 U64 extended_payload_len_64;

	 U32 masking_key;
}FRAME_Header_s;


typedef struct _FRAME_Content_s
{
	//FRAME_Header_s header;
	 U8 header[HEADER_MAX_LEN];
	 U8 header_data_index;
	
	 U8 * payload_data;
	 U64 payload_size;
}FRAME_Content_s;

typedef enum
{
	OPC_CONTINUATION = 0x0,
        OPC_TEXT = 0x1,
        OPC_BINARY = 0x2,
        OPC_RSV3 = 0x3,
        OPC_RSV4 = 0x4,
        OPC_RSV5 = 0x5,
        OPC_RSV6 = 0x6,
        OPC_RSV7 = 0x7,
        OPC_CLOSE = 0x8,
        OPC_PING = 0x9,
        OPC_PONG = 0xA,
        OPC_CONTROL_RSVB = 0xB,
        OPC_CONTROL_RSVC = 0xC,
        OPC_CONTROL_RSVD = 0xD,
        OPC_CONTROL_RSVE = 0xE,
        OPC_CONTROL_RSVF = 0xF
}OPCODE_e;

typedef enum
{
	BPB0_OPCODE = 0x0F,
   	BPB0_RSV3 = 0x10,
    	BPB0_RSV2 = 0x20,
   	BPB0_RSV1 = 0x40,
    	BPB0_FIN = 0x80,
    	BPB1_PAYLOAD = 0x7F,
    	BPB1_MASK = 0x80
}FRAME_HEADER_MASK_e;

typedef enum 
{
            INVALID_END = 999,
            NORMAL = 1000,
            GOING_AWAY = 1001,
            PROTOCOL_ERROR = 1002,
            UNSUPPORTED_DATA = 1003,
            RSV_ADHOC_1 = 1004,
            NO_STATUS = 1005,
            ABNORMAL_CLOSE = 1006,
            INVALID_PAYLOAD = 1007,
            POLICY_VIOLATION = 1008,
            MESSAGE_TOO_BIG = 1009,
            EXTENSION_REQUIRE = 1010,
            INTERNAL_ENDPOINT_ERROR = 1011,
            RSV_ADHOC_2 = 1012,
            RSV_ADHOC_3 = 1013,
            RSV_ADHOC_4 = 1014,
            TLS_HANDSHAKE = 1015,
            RSV_START = 1016,
            RSV_END = 2999,
            INVALID_START = 5000
 }STATUS_CODE_e;

typedef enum
{
	BASIC_PAYLOAD_16BIT_CODE = 0x7E, // 126
	BASIC_PAYLOAD_64BIT_CODE = 0x7F // 127
}PAYLOAD_LEN_TYPE_e;

typedef enum
{
	PAYLOAD_SIZE_BASIC = 125, 
	PAYLOAD_SIZE_EXTENDED = 0xFFFF,// 2^16, 65535
	PAYLOAD_SIZE_JUMBO = 0x7FFFFFFFFFFFFFFFLL// 2 ^ 63
}LIMIT_e;

typedef enum
{
	BINARRY,
	UTF_8_STR
}PAYLOAD_TYPE_e;


#endif
