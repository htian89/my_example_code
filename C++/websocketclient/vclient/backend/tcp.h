#ifndef TCP_H
#define TCP_H

#include <sys/types.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../common/ds_vclient.h"
//#include "vaccess.h"

typedef struct tcp_connection
{
	int fd;
	SSL_CTX *ctx;
	SSL *ssl;
} TcpConnection;

typedef struct tcp_header TcpHeader;
struct tcp_header
{
	char *name;
	char *value;
	TcpHeader *next; /* FIXME: this is ugly; need cons cell. */
};

typedef struct tcp_response
{
	int majorVersion;
	int minorVersion;
	int statusCode;
	char *statusMessage;
	TcpHeader *header;
} TcpResponse;

typedef enum FapvclientMsgType {
FAP_VCLIENT_MSG_PRESENTATION_START = 1,
FAP_VCLIENT_MSG_PRESENTATION_STOP,
FAP_VCLIENT_MSG_ENUM_END
} FapvclientMsgType;

typedef struct _FapvclientMsg {
int32_t type;
char ip[16];
u_int16_t port;

} FapvclientMsg;


#ifdef __cplusplus
extern "C" {
#endif

    extern TcpConnection *TcpConnect(NETWORK *network);
	extern void TcpDisconnect(TcpConnection *connection, int isHttps);
	extern int TcpSendSocket(TcpConnection *connection, int isHttps, const char *msg);
	extern int TcpReceiveSocket(TcpConnection *connection, int isHttps, char *buffer, int *bufferLength);

#ifdef __cplusplus
}
#endif

#endif // TCP_H
