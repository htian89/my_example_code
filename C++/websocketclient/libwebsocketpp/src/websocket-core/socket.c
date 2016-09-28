#include "socket.h"
#include "common.h"
#include  <errno.h>

int socket_send(int socketfd, char * buffer, int len)
{
	int sent = 0;

	while (len > 0)
	{
		sent = send(socketfd, buffer, len, 0);
		if (sent == -1)
		{
			 perror("socket send fail!\n");
			 return RS_FAIL;
		}

		len -= sent;
		buffer += sent;
	}

	return sent;
}

int socket_recv(int socketfd, char *buffer, int len)
{
	int recvt = 0;
	int rs = 1;

	while (rs )
	{
		recvt = recv(socketfd, buffer, len, 0);
		if (recvt == -1)
		{
			if (errno == EAGAIN)
			{
				continue;
			}
			else
			{
				perror("socket recv fail!\n");
				return RS_FAIL;
			}
		}
		else if (recvt == 0)
		{
			DBG_PRINT("TCP May be closed\n");
			return -1;
		}
		//WARNIF(recvt == len, "there is some data in the tcp buffer yet\n");
		rs = 0;
	}

	return recvt;
}




