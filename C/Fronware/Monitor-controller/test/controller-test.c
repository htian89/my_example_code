#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <linux/netlink.h>

#include <cJSON.h>
#include <types.h>

#define SEND_BUF_SIZE 8192

struct tcp_client
{
	int sockfd_client;
	uint16_t serverport;
	uint8_t serverip[32];
    uint8_t *send_buf;
    uint8_t *recv_buf;
};
typedef struct tcp_client TCPClient;

int tcp_client_connect(TCPClient *client)
{
	struct sockaddr_in server_addr;
	int server_addr_len;
	int sockfd;
	const int val = 1;
	int ret;
	if((sockfd= socket(AF_INET,SOCK_STREAM,0))< 0){
		LLOG_ERR("create sockfd_usbip_client error!\n");
		return 1;
	}
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5257);
	server_addr.sin_addr.s_addr = inet_addr("192.168.11.10");
    LLOG("size of s_addr = %d", sizeof(server_addr.sin_addr.s_addr));
	server_addr_len = sizeof(server_addr);

	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
	if (ret < 0)
		LLOG_ERR("setsockopt: TCP_NODELAY\n");

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (ret < 0)
		LLOG_ERR("setsockopt: SO_KEEPALIVE\n");

	if(connect(sockfd,(struct sockaddr *) & server_addr,server_addr_len)<0)
	{
		LLOG_ERR(" can't connect to IP\n");
		close(sockfd);
		return -1;
	}
	client->sockfd_client = sockfd;
	return ret;
}

int tcp_client_recv(TCPClient *client)
{
    int ret;
    LLOG("");
    ret = recv(client->sockfd_client, client->recv_buf, SEND_BUF_SIZE, 0);
    if(ret < 0)
    {
        LLOG_ERR("recv faild , ret = %d\n", ret);
    }
    return ret;
}

int tcp_client_send(TCPClient *client, char *send_buf, int len)
{
    int ret;
    LLOG("");
    ret = send(client->sockfd_client, send_buf, len, 0);
    if(ret != len)
    {
        LLOG_ERR("send faild , ret = %d\n", ret);
    }
    return ret;
}

int main(int argc, char *argv[])
{
    cJSON *in,*out;
    char *action;
    char *out_msg, *in_msg;
    TCPClient *client = (TCPClient *)malloc(sizeof(TCPClient));
    client->send_buf = calloc(1, SEND_BUF_SIZE);
    client->recv_buf = calloc(1, SEND_BUF_SIZE);
    if(tcp_client_connect(client))
    {
        LLOG_ERR("client connect to server failed\n");
        return;
    }
    
    if(argc != 2)
        exit(0);

    if(strcmp(argv[1], "connect") == 0)
    {
        out = cJSON_CreateObject();
        cJSON_AddItemToObject(out, "Action", cJSON_CreateString("ConnectToMonitor"));
        cJSON_AddItemToObject(out, "Monitor", cJSON_CreateNumber(1));
        cJSON_AddItemToObject(out, "Mode", cJSON_CreateNumber(0));
        cJSON_AddItemToObject(out, "TargetIp", cJSON_CreateString("10.10.20.150"));
        cJSON_AddItemToObject(out, "TargetPort", cJSON_CreateNumber(50002));
        out_msg = cJSON_Print(out);
        printf("%s\n", out_msg);
        tcp_client_send(client, out_msg, strlen(out_msg));
        free(out_msg);
        cJSON_Delete(out);

        tcp_client_recv(client);
        in = cJSON_Parse(client->recv_buf);
        in_msg = cJSON_Print(in);
        printf("%s\n", in_msg);
        free(in_msg);
        cJSON_Delete(in);

        out = cJSON_CreateObject();
        cJSON_AddItemToObject(out, "Action", cJSON_CreateString("ConnectToMonitor"));
        cJSON_AddItemToObject(out, "Monitor", cJSON_CreateNumber(2));
        cJSON_AddItemToObject(out, "Mode", cJSON_CreateNumber(1));
        cJSON_AddItemToObject(out, "TargetIp", cJSON_CreateString("192.168.29.64"));
        cJSON_AddItemToObject(out, "TargetPort", cJSON_CreateNumber(11100));
        out_msg = cJSON_Print(out);
        printf("%s\n", out_msg);
        tcp_client_send(client, out_msg, strlen(out_msg));
        free(out_msg);
        cJSON_Delete(out);

        tcp_client_recv(client);
        in = cJSON_Parse(client->recv_buf);
        in_msg = cJSON_Print(in);
        printf("%s\n", in_msg);
        free(in_msg);
        cJSON_Delete(in);
    }else if(strcmp(argv[1], "getinfo") == 0){
        out = cJSON_CreateObject();
        cJSON_AddItemToObject(out, "Action", cJSON_CreateString("GetMonitorsInfo"));
        out_msg = cJSON_Print(out);
        printf("%s\n", out_msg);
        tcp_client_send(client, out_msg, strlen(out_msg));
        free(out_msg);
        cJSON_Delete(out);

        tcp_client_recv(client);
        in = cJSON_Parse(client->recv_buf);
        in_msg = cJSON_Print(in);
        printf("%s\n", in_msg);
        free(in_msg);
        cJSON_Delete(in);
    }else if(strcmp(argv[1], "disconnect") == 0){
        out = cJSON_CreateObject();
        cJSON_AddItemToObject(out, "Action", cJSON_CreateString("DisconnectMonitor"));
        cJSON_AddItemToObject(out, "Monitor", cJSON_CreateNumber(1));
        out_msg = cJSON_Print(out);
        printf("%s\n", out_msg);
        tcp_client_send(client, out_msg, strlen(out_msg));
        free(out_msg);
        cJSON_Delete(out);

        tcp_client_recv(client);
        in = cJSON_Parse(client->recv_buf);
        in_msg = cJSON_Print(in);
        printf("%s\n", in_msg);
        free(in_msg);
        cJSON_Delete(in);
    }else if(strcmp(argv[1], "none") == 0){
        out = cJSON_CreateObject();
        cJSON_AddItemToObject(out, "AAAction", cJSON_CreateString("DisconnectMonitor"));
        out_msg = cJSON_Print(out);
        printf("%s\n", out_msg);
        tcp_client_send(client, out_msg, strlen(out_msg));
        free(out_msg);
        cJSON_Delete(out);
    }else if(strcmp(argv[1], "other") == 0){
        out_msg = malloc(24);
        tcp_client_send(client, out_msg, 24);
        free(out_msg);
    }
    return 0; 
}
