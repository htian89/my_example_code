#include "frame.h"
#include <arpa/inet.h> 


void print_data_hex(unsigned char *buf, int len, char *name)
{
	int i = 0;
	if (!debug_enable())
		return;
	printf("\033[33m###############[---%s---]########################\n", name);
	for (i = 0; i < len; i++)
	{
		if (((i+1) %32) == 0)
			printf("\n");
		printf("%02x ", buf[i]);
	}
	printf("\n#############################################\033[m\n");
}



static U8 opcode_reserved(OPCODE_e eOpCode)
{
	 return (eOpCode >= OPC_RSV3 && eOpCode <= OPC_RSV7) || 
                (eOpCode >= OPC_CONTROL_RSVB && eOpCode <= OPC_CONTROL_RSVF);
}
static RS_s set_header_bit(FRAME_Handle hFrame, U8 bit, U8 byte, U8 value)
{
	if (value == 1)
	{
		hFrame->content.header[byte] |= bit;
	}
	else
	{
		hFrame->content.header[byte] &= (0xFF ^ bit);
	}	
	return RS_OK;
}

static U8 get_header_bit(FRAME_Handle hFrame, U8 bit, U8 byte)
{
	return ((hFrame->content.header[byte]) & (bit));
}

static RS_s set_fin(FRAME_Handle hFrame, U8 fin)
{
	return set_header_bit(hFrame, BPB0_FIN, 0, fin);
}

static U8 get_fin(FRAME_Handle hFrame)
{
	return get_header_bit(hFrame, BPB0_FIN, 0);
}


static RS_s set_RSV1(FRAME_Handle hFrame, U8 rsv1)
{
	return set_header_bit(hFrame, BPB0_RSV1, 0, rsv1);
}

static U8 get_RSV1(FRAME_Handle hFrame)
{
	return get_header_bit(hFrame, BPB0_RSV1, 0);
}

static RS_s set_RSV2(FRAME_Handle hFrame, U8 rsv2)
{
	return set_header_bit(hFrame, BPB0_RSV2, 0, rsv2);
}

static U8 get_RSV2(FRAME_Handle hFrame)
{
	return get_header_bit(hFrame, BPB0_RSV2, 0);
}


static RS_s set_RSV3(FRAME_Handle hFrame, U8 rsv3)
{
	return set_header_bit(hFrame, BPB0_RSV3, 0, rsv3);
}

static U8 get_RSV3(FRAME_Handle hFrame)
{
	return get_header_bit(hFrame, BPB0_RSV3, 0);
}

static RS_s set_opcode(FRAME_Handle hFrame, U8 opCode)
{
	hFrame->content.header[0] &= (0xFF ^ BPB0_OPCODE);
	hFrame->content.header[0] |= opCode;
	return RS_OK;
//	return set_header_bit(hFrame, BPB0_OPCODE, 0, opCode);
}

static U8 get_opcode(FRAME_Handle hFrame)
{
//	return (hFrame->content.header[0]) & (BPB0_OPCODE);
	return get_header_bit(hFrame, BPB0_OPCODE, 0);
}

static RS_s set_basic_size(FRAME_Handle hFrame, U8 len)
{
	hFrame->content.header[1] |= (len & BPB1_PAYLOAD);
	return RS_OK;
}

static U8 get_basic_size(FRAME_Handle hFrame)
{
	return get_header_bit(hFrame, BPB1_PAYLOAD, 1);
}

static RS_s set_payload_extented_len_16(FRAME_Handle hFrame, U16 size)
{
	//hFrame->content.header.extended_payload_len_16 = htons(size);
	U16 *extend_len = (U16 *)&hFrame->content.header[BASIC_HEADER_LEN];
	*extend_len = htons(size);
	return RS_OK;
}

static U16 get_payload_extented_len_16(FRAME_Handle hFrame)
{
	//hFrame->content.header.extended_payload_len_16 = htons(size);
//	U16 *extend_len = (U16 *)&hFrame->content.header[BASIC_HEADER_LEN];
//	U16 extend_len = (hFrame->content.header[BASIC_HEADER_LEN] << 8) & 0xF0;
//	extend_len |= (hFrame->content.header[BASIC_HEADER_LEN + 1]);

	U16 *extend_len_net = (U16 *)&hFrame->content.header[BASIC_HEADER_LEN];
	U16 extend_len = ntohs(*extend_len_net);

	return extend_len;
}

static U64 set_payload_extented_len_64(FRAME_Handle hFrame, U64 size)
{
//	U16 extend_len = (hFrame->content.header[BASIC_HEADER_LEN] << 8) & 0xF0;
//	extend_len |= (hFrame->content.header[BASIC_HEADER_LEN + 1]);
	U64 *extend_len = (U64 *)&hFrame->content.header[BASIC_HEADER_LEN];
	*extend_len = htonl(size);
	
	return RS_OK;
}

static U64 get_payload_extented_len_64(FRAME_Handle hFrame)
{
//	U64 *extend_len = (U64 *)&hFrame->content.header[BASIC_HEADER_LEN];
	U64 *extend_len_net = (U64 *)&hFrame->content.header[BASIC_HEADER_LEN];
	U64 extend_len = ntohl(*extend_len_net);
	
	return extend_len;
}

static U32 gernate_mask_key(void)
{
	return my_rand(0xFFFFFFFF);
}

static RS_s set_mask(FRAME_Handle hFrame, U8 mask)
{
	//hFrame->content.header[1] = mask;
	return set_header_bit(hFrame, BPB1_MASK, 1, mask);
}

static U8 get_mask(FRAME_Handle hFrame)
{
	return get_header_bit(hFrame, BPB1_MASK, 1);
}


static U32 get_header_len(FRAME_Handle hFrame)
{
	U32 temp = 2;

	if (get_mask(hFrame))
	{
		temp += 4;
	}

	if (get_basic_size(hFrame) == 126)
	{
		temp += 2;
	}
	else if (get_basic_size(hFrame) == 127)
	{
		temp += 8;
	}

	return temp;
}

static RS_s set_mask_key(FRAME_Handle hFrame)
{
	U32 *msking_key = (U32 *)&hFrame->content.header[get_header_len(hFrame) - 4];
	*msking_key = gernate_mask_key();
	return RS_OK;
}

static void update_need_bytes(FRAME_Handle hFrame, U8 value)
{
	hFrame->need_bytes = get_header_len(hFrame) - value;
}

static int is_control(FRAME_Handle hFrame)
{
	return (hFrame->content.header[0] & BPB0_OPCODE) >= OPC_CLOSE;
}

static int is_close(FRAME_Handle hFrame)
{
	return (hFrame->content.header[0] & BPB0_OPCODE) == OPC_CLOSE;
}

static RS_s set_payload_helper(FRAME_Handle hFrame, U64 size)
{
	RETURNIF(size > PAYLOAD_MAX_SIZE, RS_ERROR, "requested payload is over implementation defined limit");
	RETURNIF(is_control(hFrame) && (size > PAYLOAD_SIZE_BASIC), RS_ERROR, "control frames can not have large payloads");

	if (size <= PAYLOAD_SIZE_BASIC)
	{
		set_basic_size(hFrame, (U8)size);
	}
	else if (size <= PAYLOAD_SIZE_EXTENDED)
	{
		set_basic_size(hFrame, BASIC_PAYLOAD_16BIT_CODE);
		set_payload_extented_len_16(hFrame, size);
	}
	else if (size <= PAYLOAD_SIZE_JUMBO)
	{
		set_basic_size(hFrame, BASIC_PAYLOAD_64BIT_CODE);
		set_payload_extented_len_64(hFrame, size);
	}

	if (get_mask(hFrame))
	{
		set_mask_key(hFrame);
	}
	
	return RS_OK;
}

static RS_s set_payload(FRAME_Handle hFrame, U8 *payload, U64 len)
{
	RS_s ret = RS_OK;
	
	ret = set_payload_helper(hFrame, len);
	RETURNIF(ret != RS_OK, ret, "");
	
	hFrame->content.payload_size = len;
	hFrame->content.payload_data = my_malloc(len);
	RETURNIF(hFrame->content.payload_data == NULL, RS_FAIL, "");
	
	memcpy(hFrame->content.payload_data, payload, len);
//	DBG_PRINT("%s\n", hFrame->content.payload_data);

	return RS_OK;
}

static RS_s process_payload(FRAME_Handle hFrame)
{
	if (get_mask(hFrame))
	{
		U8 *masking_key = (U8 *)&hFrame->content.header[get_header_len(hFrame) - 4];
		U8 *payload = hFrame->content.payload_data;
		U64 i = 0;

		for (i = 0; i <  hFrame->content.payload_size; i++)
		{
			payload[i] = (payload[i] ^ masking_key[i%4]);
		}
	}

	return RS_OK;
}

static RS_s prepare_frame(FRAME_Handle hFrame, OPCODE_e eOpcode, U8 *payload, U64 len)
{
	set_fin(hFrame, 1);
	set_RSV1(hFrame, 0);
	set_RSV2(hFrame, 0);
	set_RSV3(hFrame, 0);

	set_mask(hFrame, 1);

	set_opcode(hFrame, eOpcode);

	set_payload(hFrame, payload, len);

	process_payload(hFrame);

	return RS_OK;
}


static RS_s __prepare_data_frame(FRAME_Handle hFrame, PAYLOAD_TYPE_e eType, U8 *payload, U64 len)
{
	OPCODE_e eOpcode;
	eOpcode = (eType == UTF_8_STR) ? OPC_TEXT : OPC_BINARY;

 	return prepare_frame(hFrame, eOpcode, payload, len);
}

static RS_s __prepare_close_frame(FRAME_Handle hFrame, U8 *reason, U32 len)
{
	U8 *payload = my_malloc(len + 2);
	U16 *temp = (U16 *)payload;
	
	*temp = htons(NO_STATUS);
	snprintf(payload+2,len, "%s", reason);
	prepare_frame(hFrame, OPC_CLOSE, reason, len);
}

static RS_s __parse_frame(FRAME_Handle hFrame, U8 *frame, U32 len)
{
	return RS_OK;
}

RS_s __reset(FRAME_Handle hFrame)
{
	if (hFrame->content.payload_data)
	{
		my_free(hFrame->content.payload_data);
	}
	
	memset(&hFrame->content, 0, sizeof(FRAME_Content_s));
	hFrame->need_bytes = 0;
	
	return RS_OK;
}

RS_s __process_close_frame(FRAME_Handle hFrame)
{
	//RETURNIF(!is_close(hFrame), RS_ERROR, "This is not close frame");
	if (is_close(hFrame))
	{
		//get_reason(hFrame);
		//get_code(hFrame);
	}
	else
	{
		DBG_PRINT("%d\n", hFrame->content.payload_size);
//		DBG_PRINT("%s\n", hFrame->content.payload_data);
		return RS_ERROR;
	}

	return RS_OK;
}


static RS_s validate_basic_header(FRAME_Handle hFrame)
{
	RETURNIF((is_control(hFrame) && get_basic_size(hFrame) > PAYLOAD_SIZE_BASIC), RS_ERROR, "Control frame is large");
	RETURNIF((get_RSV1(hFrame) ||  get_RSV2(hFrame) || get_RSV3(hFrame)), RS_ERROR, "Reserved bit used");
	RETURNIF((opcode_reserved(get_opcode(hFrame))), RS_ERROR, "Reserved opcode used");
	RETURNIF((is_control(hFrame) && !get_fin(hFrame)), RS_ERROR, "Fragmented control message");
	RETURNIF(get_mask(hFrame), RS_ERROR, "Server frame must not mask");

	return RS_OK;
}

static RS_s process_basic_header(FRAME_Handle hFrame)
{
//	print_frame(hFrame);
	RETURNIF(validate_basic_header(hFrame) != RS_OK, RS_ERROR, "");
	DBG_PRINT("%d\n", get_header_len(hFrame) );

	hFrame->need_bytes = get_header_len(hFrame) - BASIC_HEADER_LEN;
	return RS_OK;
}

static RS_s process_extend_header(FRAME_Handle hFrame)
{
	U8 basic_size  = get_basic_size(hFrame);
	U64 payload_size;
	 int mask_index = BASIC_HEADER_LEN;

	 DBG_PRINT("basic_size :%d\n", basic_size);

//	 print_data_hex(hFrame->content.header, 14, "header");

	if (basic_size <= PAYLOAD_SIZE_BASIC)
	{
		payload_size = basic_size;
	}
	else if (basic_size == BASIC_PAYLOAD_16BIT_CODE)
	{
		payload_size = get_payload_extented_len_16(hFrame);
		DBG_PRINT("BASIC_PAYLOAD_16BIT_CODE: payload_size :%d\n", payload_size);
		RETURNIF(payload_size < basic_size, RS_ERROR, "payload length not minimally encoded. Using 16 bit form for payload size");
		mask_index += 2;
	}
	else if (basic_size == BASIC_PAYLOAD_64BIT_CODE)
	{
		payload_size = get_payload_extented_len_64(hFrame);
		RETURNIF(payload_size < PAYLOAD_SIZE_EXTENDED, RS_ERROR, "payload length not minimally encoded");
		mask_index += 8;
	}

	if (get_mask(hFrame) == 0)
	{
		
	}
	else
	{

	}
	
	RETURNIF(payload_size > PAYLOAD_MAX_SIZE, RS_ERROR, "payload is so large");

	hFrame->need_bytes = payload_size;
	DBG_PRINT("need_bytes:%d\n", hFrame->need_bytes);
	
	return RS_OK;
}


RS_s frame_init(FRAME_Handle *hFrame)
{
	FRAME_Handle _hFrame = NULL;

	_hFrame = (FRAME_Handle)my_malloc(sizeof(FRAME_s));
	RETURNIF(_hFrame == NULL, RS_ERROR, "");

	_hFrame->parse_frame = __parse_frame;
	_hFrame->prepare_data_frame = __prepare_data_frame;
	_hFrame->prepare_close_frame = __prepare_close_frame;
	_hFrame->process_close_frame = __process_close_frame;
	_hFrame->reset = __reset;
	
	*hFrame = _hFrame;

	return RS_OK;
}

RS_s frame_uninit(FRAME_Handle hFrame)
{
	if (hFrame->content.payload_data)
	{
		my_free(hFrame->content.payload_data);
	}

	my_free((char *)hFrame);
}

///// test
RS_s send_frame(int socketfd, FRAME_Handle hFrame)
{
//	U8 header[16];
	U8 *payload = NULL;
	U64 len;
	RS_s ret = RS_OK;

	payload = hFrame->content.payload_data;
	len =  hFrame->content.payload_size;

	ret = socket_send(socketfd, hFrame->content.header, get_header_len(hFrame));

	ret = socket_send(socketfd, payload, len);

	return RS_OK;
}


RS_s recv_frame(int socketfd, FRAME_Handle hFrame)
{
	U8 *payload = NULL;
	U64 len;
	RS_s ret = RS_OK;
	
	payload = hFrame->content.payload_data;
	len =  hFrame->content.payload_size;
	hFrame->content.header_data_index = 0;
	do 
	{
		hFrame->need_bytes = get_header_len(hFrame);
		DBG_PRINT("1. need bytes :%d\n", hFrame->need_bytes);
		len = socket_recv(socketfd, hFrame->content.header, hFrame->need_bytes);
		if (len == -1) 
		{
			return RS_SOCKETCLOSE;
		}
		hFrame->content.header_data_index += len;
		hFrame->need_bytes -= (U8)len;
		if (hFrame->need_bytes == 0)
		{
  	        	CHECK_RUN(process_basic_header(hFrame));
		}

		DBG_PRINT("need_bytes:%d\n", hFrame->need_bytes);
		if (hFrame->need_bytes)
		{
			len = socket_recv(socketfd, hFrame->content.header+hFrame->content.header_data_index, hFrame->need_bytes);
			if (len == -1) 
			{
				return RS_SOCKETCLOSE;
			}
			hFrame->need_bytes -= (U8)len;
		}
		CHECK_RUN(process_extend_header(hFrame));

		DBG_PRINT("need_bytes:%d\n", hFrame->need_bytes);
		if (hFrame->need_bytes && hFrame->need_bytes <= PAYLOAD_MAX_SIZE)
		{
			U8 *payload = NULL;

			payload = my_malloc(hFrame->need_bytes);
			hFrame->content.payload_data = payload;
			hFrame->content.payload_size = hFrame->need_bytes;
			
			while (hFrame->need_bytes > 0)
			{
				len = socket_recv(socketfd, payload, hFrame->need_bytes);
				if (len < 0) 
				{
					my_free(hFrame->content.payload_data);
					return RS_SOCKETCLOSE;
				}
				hFrame->need_bytes -= len;
				payload += len;
			}
			process_payload(hFrame);
		}
		return RS_OK;
	}while (0);

	return RS_ERROR;
}
/*
typedef struct _FRAME_Header_s
{
	 U8 FIN:1;
	 U8 RSV1:1;
	 U8 RSV2:1;
	 U8 RSV3:1;
	 U8 opcode:4;

	 U8 MASK:1;
	 U8 payload_len:7;

	 U16 extended_payload_len_16;
	 U64 extended_payload_len_64;

	 U32 masking_key;
}FRAME_Header_s;
*/
RS_s print_frame(FRAME_Handle hFrame)
{
	int i ;

	if (!debug_enable)
		return RS_OK;
	
	DBG_PRINT("@@@@@@@@@@@@@@@@@@@@@@@@@\nframe raw content:\n");
	U8  *p = (U8 *)&(hFrame->content.header);
	for (i= 0; i < HEADER_MAX_LEN; i++)
	{
		printf("%02x ", p[i]);
		if ((i+1)%32 == 0)
		{
			printf("\n");
		}
	}
	printf("\n");
	printf("content.payload_size:%d\n", hFrame->content.payload_size);

	p = hFrame->content.payload_data;
	for (i = 0; i < hFrame->content.payload_size; i ++)
	{
		printf("%02x ", p[i]);
		if ((i+1)%32 == 0)
		{
			printf("\n");
		}
	}
	printf("\n");
	DBG_PRINT("frame raw data end:\n");
}
