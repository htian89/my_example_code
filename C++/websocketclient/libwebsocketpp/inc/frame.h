#ifndef __FRAME_H__
#define __FRAME_H__

#include "common.h"
#include "frame_constant.h"

#define FRAME_HANDLE struct _FRAME_s *

#define PAYLOAD_MAX_SIZE (100 * 1024 * 1024)  //100MB

typedef struct _FRAME_s
{
	FRAME_Content_s content;
	U64 need_bytes;
	RS_s (*prepare_data_frame)(FRAME_HANDLE, PAYLOAD_TYPE_e eType, U8 *payload, U64 len);
	RS_s (*prepare_close_frame)(FRAME_HANDLE, U8 *reason, U32 len);
	RS_s (*process_close_frame)(FRAME_HANDLE);
	RS_s (*reset)(FRAME_HANDLE);
	RS_s (*parse_frame)(FRAME_HANDLE, U8 *frame, U32 len);

}FRAME_s, *FRAME_Handle;

RS_s frame_init(FRAME_Handle *hFrame);
RS_s frame_uninit(FRAME_Handle hFrame);

RS_s send_frame(int socketfd, FRAME_Handle hFrame);
RS_s recv_frame(int socketfd, FRAME_Handle hFrame);

RS_s print_frame(FRAME_Handle hFrame);

#endif
