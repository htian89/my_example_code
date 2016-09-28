#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#endif

#include <errno.h>
#include "tcp.h"
#include <time.h>

#include "../common/log.h"

static int TcpErrorToErrno (int err)
{
    /* Error codes taken from RFC2068. */
    LOG_ERR("TcpErrorToErrno errno:%d", err);
    switch (err)
    {
    case -1:    /* system error */
        return errno;
    case -200:  /* OK */
    case -201:  /* Created */
    case -202:  /* Accepted */
    case -203:  /* Non-Authoritative Information */
    case -204:  /* No Content */
    case -205:  /* Reset Content */
    case -206:  /* Partial Content */
        return 0;
    case -400:  /* Bad Request */
        // printf("http_error_to_errno: 400 bad request");
        return EIO;
    case -401:  /* Unauthorized */
        // printf("http_error_to_errno: 401 unauthorized");
        return EACCES;
    case -403:  /* Forbidden */
        // printf("http_error_to_errno: 403 forbidden");
        return EACCES;
    case -404:  /* Not Found */
        // printf("http_error_to_errno: 404 not found");
        return ENOENT;
    case -411:  /* Length Required */
        // printf("http_error_to_errno: 411 length required");
        return EIO;
    case -413:  /* Request Entity Too Large */
        // printf("http_error_to_errno: 413 request entity too large");
        return EIO;
    case -505:  /* HTTP Version Not Supported       */
        // printf("http_error_to_errno: 413 HTTP version not supported");
        return EIO;
    case -100:  /* Continue */
    case -101:  /* Switching Protocols */
    case -300:  /* Multiple Choices */
    case -301:  /* Moved Permanently */
    case -302:  /* Moved Temporarily */
    case -303:  /* See Other */
    case -304:  /* Not Modified */
    case -305:  /* Use Proxy */
    case -402:  /* Payment Required */
    case -405:  /* Method Not Allowed */
    case -406:  /* Not Acceptable */
    case -407:  /* Proxy Autentication Required */
    case -408:  /* Request Timeout */
    case -409:  /* Conflict */
    case -410:  /* Gone */
    case -412:  /* Precondition Failed */
    case -414:  /* Request-URI Too Long */
    case -415:  /* Unsupported Media Type */
    case -500:  /* Internal Server Error */
    case -501:  /* Not Implemented */
    case -502:  /* Bad Gateway */
    case -503:  /* Service Unavailable */
    case -504:  /* Gateway Timeout */
        // printf("http_error_to_errno: HTTP error %d", err);
        return EIO;
    default:
        // printf("http_error_to_errno: unknown error %d", err);
        return EIO;
    }
}

static int NoBlockRecv(int fd, char *buffer, int len, int timeOut)
{
    fd_set fds;
    struct timeval timeout = {timeOut,0}; //select等待3秒，3秒轮询，要非阻塞就置0
    FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化
    FD_SET(fd,&fds); //添加描述符
    int maxfdp = 0;
    maxfdp = fd>maxfdp ? fd:maxfdp;

#ifdef unix
    maxfdp++;
#endif

    switch(select(maxfdp, &fds, NULL, NULL, &timeout))
    {
    case 0:
        LOG_ERR("%s","select return 0, time out");//timeout
        return 0;
    case -1:
#ifdef WIN32
        LOG_ERR("select return -1, time out. errno:%d", WSAGetLastError()); //error
#endif
        return -1;
    default:
        if(FD_ISSET(fd, &fds))
            return recv(fd, buffer, len, 0);
    }
    return 0;
}

static long ReadAll(TcpConnection *tcpConnection, int isHttps, char *buf, size_t len)
{
    size_t n, r;
    int m;
    char *rbuf = buf;

    //    char *p;

    r = len;
    for (n = 0; n < len; n += m)
    {
        // printf("read (%d, %p, %d) ...", fd, rbuf + n, len - n);
        if (isHttps)
            m = SSL_read(tcpConnection->ssl, rbuf + n, len - n);
        else
            m = NoBlockRecv(tcpConnection->fd, rbuf+n, len-n, 30);
        //			m = recv(tcpConnection->fd, rbuf + n, len - n, 0);

        //        p = rbuf + n;
        //        int i = 0;
        //        while (p != NULL && i < len - n)
        //        {
        //             printf("%c", *p);
        //             printf("%x ", *p);
        //            p++;
        //            i++;
        //        }

        //      printf("... = %d", m);
        if (m == 0)
        {
            r = 0;
            break;
        }
        else if (m == -1)
        {
            if (errno != EAGAIN)
            {
                r = -1;
                break;
            }
            else
                m = 0;
        }
    }

    return r;
}

static long ReadUntil(TcpConnection *tcpConnection, int isHttps, int ch, char **data)
{
    char *buf, *buf2;
    long n, len, bufSize;

    *data = NULL;

    bufSize = 100;
    buf = (char *)malloc(bufSize);
    if (buf == NULL)
    {
#ifdef DEBUG_MODE
        printf("read_until: out of memory");
#endif
        return -1;
    }

    len = 0;
    while ((n = ReadAll(tcpConnection, isHttps, buf + len, 1)) == 1)
    {
        if (ch != 'x')
        {
            if (buf[len++] == ch)
                break;
        }
        else
            len++;

        if (len + 1 == bufSize)
        {
            bufSize *= 2;
            buf2 = (char *)realloc(buf, bufSize);
            if (buf2 == NULL)
            {
#ifdef DEBUG_MODE
                printf("read_until: realloc failed");
#endif
                free(buf);
                return -1;
            }

            buf = buf2;
        }
    }

    if (ch == 'x')
    {
        buf2 = (char *)realloc(buf, len + 1);
        if (buf2 == NULL)
            printf("read_until: realloc: shrink failed"); /* not fatal */
        else
            buf = buf2;

        len++;

        *data = buf;
        return len;
    }

    if (n <= 0)
    {
        free(buf);
        if (n == 0)
            printf("read_until: closed");
        else
            printf("read_until: read error: %s", strerror(errno));
        return n;
    }

    /* Shrink to minimum size + 1 in case someone wants to add a NUL. */
    buf2 = (char *)realloc(buf, len + 1);
    if (buf2 == NULL)
        printf("read_until: realloc: shrink failed"); /* not fatal */
    else
        buf = buf2;

    *data = buf;
    return len;
}

static long TcpParseHeader(TcpConnection *tcpConnection, int isHttps, TcpHeader **header, int* i_count)
{
    char buf[2];
    char *data;
    TcpHeader *h;
    size_t len;
    long n;
    static size_t real_data_len;
    //static int i = 0;
    if(NULL == i_count )
    {
        printf("TcpParseHeader failed:NULL == i_count\n");
    }
    int i = *i_count;

    *header = NULL;
    n = ReadAll(tcpConnection, isHttps, buf, 2);
    if (n <= 0)
        return n;
    if (buf[0] == '\r' && buf[1] == '\n')
    {
        i++;
        if (i == 2)
        {
            i = 0;
            return n;
        }
    }

    h = (TcpHeader *)malloc(sizeof(TcpHeader));
    if (h == NULL)
    {
        // printf("parse_header: malloc failed");
        return -1;
    }
    *header = h;
    h->name = NULL;
    h->value = NULL;

    if (i != 1)
    {
        n = ReadUntil(tcpConnection, isHttps, ':', &data);
        if (n <= 0)
            return n;
        data = (char *)realloc(data, n + 2);
        if (data == NULL)
        {
            // printf("parse_header: realloc failed");
            return -1;
        }
        memmove(data + 2, data, n);
        memcpy(data, buf, 2);
        n += 2;
        data[n - 1] = 0;
        h->name = strdup(data);
        len = n;
        n = ReadUntil(tcpConnection, isHttps, '\r', &data);
    }
    else
    {
//        n = ReadUntil(tcpConnection, isHttps, 'x', &data);
        data = (char *)calloc(1, real_data_len);
        n = ReadAll(tcpConnection, isHttps, data, real_data_len);
        real_data_len = 0;
        len = 0;
        h->next = NULL;
    }

    if (n <= 0)
        return n;
    data[n - 1] = '\0';
    h->value = (char *)calloc(1, n);
    memcpy(h->value, data, n);
    if(h->name)
    {
        if (strcmp(h->name, "Content-Length") == 0)
            real_data_len = atoi(h->value);
    }

    len += n;

    if (i == 1)
    {
        free(data);
        data = NULL;
        i = 0;
        return len;
    }

    n = ReadUntil(tcpConnection, isHttps, '\n', &data);
    if (n <= 0)
        return n;
    free(data);
    if (n != 1)
    {
        // printf("parse_header: invalid line ending");
        return -1;
    }
    len += n;

    // printf("parse_header: %s:%s", h->name, h->value);
    *i_count = i;
    n = TcpParseHeader(tcpConnection, isHttps, &h->next, i_count);
    if (n <= 0)
        return n;
    len += n;

    return len;
}

static void TcpDestroyHeader(TcpHeader *header)
{
    if (header == NULL)
        return;

    TcpDestroyHeader(header->next);

    if (header->name)
        free(header->name);
    if (header->value)
        free(header->value);
    free(header);
}

static void TcpDestroyResponse(TcpResponse *response)
{
    if (response == NULL)
        return;

    if (response->statusMessage)
        free(response->statusMessage);
    TcpDestroyHeader(response->header);
    free(response);
}

static long TcpParseResponse(TcpConnection *tcpConnection, int isHttps, TcpResponse **response_)
{
    TcpResponse *response;
    char *data;
    size_t len;
    long n;

    *response_ = NULL;

    response = (TcpResponse *)malloc(sizeof(TcpResponse));
    if (response == NULL) {
#ifdef DEBUG_MODE
        printf("http_parse_response: out of memory");
#endif
        return -1;
    }

    response->majorVersion = -1;
    response->minorVersion = -1;
    response->statusCode = -1;
    response->statusMessage = NULL;
    response->header = NULL;

    n = ReadUntil(tcpConnection, isHttps, '/', &data);

    if (n <= 0)
    {
        free(response);
        return n;
    }
    else if (n != 5 || memcmp(data, "HTTP", 4) != 0)
    {
#ifdef DEBUG_MODE
        printf("http_parse_response: expected \"HTTP\"");
#endif
        free(data);
        free(response);
        return -1;
    }
    free(data);
    len = n;

    n = ReadUntil(tcpConnection, isHttps, '.', &data);

    if (n <= 0)
    {
        free(response);
        return n;
    }
    data[n - 1] = 0;
    response->majorVersion = atoi(data);
    // printf("http_parse_response: major version = %d", response->major_version);
    free(data);
    len += n;

    n = ReadUntil(tcpConnection, isHttps, ' ', &data);
    if (n <= 0)
    {
        free(response);
        return n;
    }
    data[n - 1] = 0;
    response->minorVersion = atoi(data);
    // printf("http_parse_response: minor version = %d", response->minor_version);
    free(data);
    len += n;

    n = ReadUntil(tcpConnection, isHttps, ' ', &data);
    if (n <= 0)
    {
        free(response);
        return n;
    }
    data[n - 1] = 0;
    response->statusCode = atoi(data);
    // printf("http_parse_response: status code = %d", response->status_code);
    free(data);
    len += n;

    n = ReadUntil(tcpConnection, isHttps, '\r', &data);
    if (n <= 0)
    {
        free(response);
        return n;
    }
    data[n - 1] = 0;
    response->statusMessage = data;
    // printf("http_parse_response: status message = \"%s\"", response->status_message);
    len += n;

    n = ReadUntil(tcpConnection, isHttps, '\n', &data);
    if (n <= 0)
    {
        TcpDestroyResponse(response);
        return n;
    }
    free(data);
    if (n != 1)
    {
#ifdef DEBUG_MODE
        printf("http_parse_request: invalid line ending");
#endif
        TcpDestroyResponse(response);
        return -1;
    }
    len += n;

    //ATTENTION:changed for thread-safe
    int i_count = 0;
    n = TcpParseHeader(tcpConnection, isHttps, &response->header, &i_count);

    if (n <= 0)
    {
        TcpDestroyResponse(response);
        return n;
    }
    len += n;

    *response_ = response;

    return len;
}

static int TcpVarifyResponse(int errorCode, TcpResponse *response)
{
    if (errorCode <= 0)
    {
        if (errorCode == 0)
            errorCode = -1;
        else
        {
#ifdef DEBUG_MODE
            printf("tunnel_in_connect: no response; error: %s\n", strerror(errno));
#endif
        }
    }
    else if (response->majorVersion != 1 || (response->minorVersion != 1
                                             && response->minorVersion != 0))
    {
#ifdef DEBUG_MODE
        printf("tunnel_in_connect: unknown HTTP version: %d.%d\n",
               response->majorVersion, response->minorVersion);
#endif
        errorCode = -1;
    }
    else if (response->statusCode != 200)
    {
#ifdef DEBUG_MODE
        printf("tunnel_in_connect: HTTP error %d\n", response->statusCode);
#endif
        errno = TcpErrorToErrno(-response->statusCode);
        errorCode = -1;
    }

    return errorCode;
}

static void Closefd(int fd)
{
#ifdef _WIN32
    closesocket(fd);
    //	WSACleanup();
#else
    close(fd);
#endif
}

static int SetAddress(struct sockaddr_in *address, const char *host, const int port)
{
    address->sin_family = PF_INET;
    address->sin_port = htons((short)port);
    address->sin_addr.s_addr = inet_addr(host);

    if (address->sin_addr.s_addr == INADDR_NONE)
    {
        struct hostent *ent;
        int ip;

        ent = gethostbyname(host);
        if (ent == 0)
            return -1;

        memcpy(&address->sin_addr.s_addr, ent->h_addr, ent->h_length);
        ip = ntohl(address->sin_addr.s_addr);
        printf("set_address: host = %d.%d.%d.%d\n",
               ip >> 24,
               (ip >> 16) & 0xff,
               (ip >>  8) & 0xff,
               ip        & 0xff);
    }

    return 0;
}

TcpConnection *TcpConnect(NETWORK *network)
{
    struct sockaddr_in *address = NULL;
    TcpConnection *tcpConnection;
    int connection_status;
    long flags;
    int error = 0, iRet, iRet2, iRetOpt;
    bool bRet;
    socklen_t len;
    fd_set rfds, wfds;
    struct timeval timeout;
#ifdef _WIN32
    //	WORD wVersionRequested;
    //	WSADATA wsaData;
    unsigned long ul = 0;
    //	int err;

    //	wVersionRequested = MAKEWORD(1, 1);
    //	err = WSAStartup(wVersionRequested, &wsaData);
    //    if(0!=err)
    //        LOG_ERR("WSAStartup failed.  %d",err);
#endif

    address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    if (SetAddress(address, network->stPresentServer.serverAddress, atoi(network->stPresentServer.port)) == -1)//if (SetAddress(address, network->presentServer, atoi(network->port)) == -1)
    {
        LOG_ERR("%s", "SetAddress Wrong\n");
        return NULL;
    }

    tcpConnection = (TcpConnection *)malloc(sizeof(TcpConnection));

    tcpConnection->fd = socket(AF_INET, SOCK_STREAM, 0);



    if (tcpConnection->fd == -1)
    {
        free(tcpConnection);
        LOG_ERR("%s", "error in socket function \n");
        return NULL;
    }
    //connect block 5sec;
#if 0
#ifdef _WIN32
    if(tcpConnection->fd == INVALID_SOCKET)
        LOG_ERR("error in socket:%d", WSAGetLastError());
    flags = ioctlsocket(tcpConnection->fd, FIONBIO, (unsigned long *)&ul);
    if (flags == SOCKET_ERROR)
    {
        LOG_ERR("error in ioctlsocket function:%d", WSAGetLastError());
        return NULL;
    }
#else
    flags = fcntl(tcpConnection->fd, F_GETFL);
    printf("fcntl(tcpConnection->fd, F_GETFL, flags):flags %x\n", flags);
    LOG_INFO("fcntl(tcpConnection->fd, F_GETFL, &flags):flags %x", flags);
    fcntl (tcpConnection->fd, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(tcpConnection->fd, F_GETFL);
    printf("fcntl(tcpConnection->fd, F_GETFL, flags):flags %x\n", flags);
#endif
#endif

#if 1
    int try_n = 1;
    for(; try_n <=10; try_n++)
    {
        iRet = connect(tcpConnection->fd, (struct sockaddr *)address,
                       sizeof( struct sockaddr_in));
        if(iRet == 0)
        {
            LOG_ERR("connect success\n");
            break;
        }
        LOG_ERR("try to connect %d times", try_n);
        sleep(1);
    }
    if(iRet == -1)
    {
        Closefd(tcpConnection->fd);
        free(tcpConnection);
        LOG_ERR("%s", "error in connect function \n");
        return NULL;
    }
#else
    iRet = connect(tcpConnection->fd, (struct sockaddr *)address,
                   sizeof( struct sockaddr_in));
    if(iRet == -1 && errno == EINPROGRESS){
        FD_ZERO(&wfds);
        FD_SET(tcpConnection->fd, &wfds);
        FD_ZERO(&rfds);
        FD_SET(tcpConnection->fd, &rfds);
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        if( select(tcpConnection->fd+1, &rfds, &wfds, NULL, &timeout) > 0){
            if(FD_ISSET(tcpConnection->fd, &wfds)){
                if(FD_ISSET(tcpConnection->fd, &rfds)){
                    iRet2 = connect(tcpConnection->fd, (struct sockaddr *)address,
                                   sizeof( struct sockaddr_in));
                    printf("error: %d\n", errno);
                    if(iRet2 == -1 && errno == EISCONN){
                        bRet = true;
                    }else if( iRet2 == 0){
                        bRet = true;
                    }else{
                        bRet = false;
                    }
//                    iRetOpt = getsockopt(tcpConnection->fd, SOL_SOCKET, SO_ERROR, &error, &len);
//                    if( iRetOpt == 0 && !error ){
//                        bRet = true;
//                    }else{
//                        bRet = false;
//                        LOG_ERR("getsockopt,return: %d, errno: %d", iRetOpt, error);
//                    }
                }else{
                    bRet = true;
                }
            }else{
                bRet = false;
                LOG_ERR("%s","FD_ISSET false");
            }
        }else{
            bRet = false;
            LOG_ERR("%s", "select (fd)<= 0");
        }
    }else if( iRet == 0){
        bRet = true;//connect success;
    }else{
        bRet = false;
        LOG_ERR("%s", "error in connect function \n");
    }
    flags = fcntl(tcpConnection->fd, F_GETFL);
    printf("fcntl(tcpConnection->fd, F_GETFL, flags):flags %x\n", flags);
    fcntl (tcpConnection->fd, F_SETFL, flags & ~O_NONBLOCK);
    LOG_INFO("fcntl(tcpConnection->fd, F_GETFL, flags):flags %x", flags);
    flags = fcntl(tcpConnection->fd, F_GETFL);
    printf("fcntl(tcpConnection->fd, F_GETFL, flags):flags %x\n", flags);
    if(!bRet){
        Closefd(tcpConnection->fd);
        free(tcpConnection);
        LOG_ERR("%s", "error in connect function \n");
        return NULL;
    }
    //connect block;
//    flags = fcntl(tcpConnection->fd, F_GETFL);
//    fcntl (tcpConnection->fd, F_SETFL, flags & ~O_NONBLOCK);
//    printf("fcntl(tcpConnection->fd, F_GETFL, flags):flags %x\n", flags);
//    if (connect(tcpConnection->fd, (struct sockaddr *)address, sizeof(struct sockaddr_in)) == -1)
//    {
//#ifdef _WIN32
//        LOG_ERR("connect failed. reason:%d", WSAGetLastError());
//#endif
//        printf("Connect fail!");
//        Closefd(tcpConnection->fd);
//        free(tcpConnection);
//        LOG_ERR("%s", "error in connect function \n");
//        return NULL;
//    }
//    flags = fcntl(tcpConnection->fd, F_GETFL);
//    printf("fcntl(tcpConnection->fd, F_GETFL, flags):flags %x\n", flags);
#endif

    if (network->stPresentServer.isHttps)//if (network->isHttps)
    {
        unsigned long err;
        const char *efunc, *elib, *ereason;

        printf("ERR_get_error = %lu\n", ERR_get_error());
        const SSL_METHOD* method = SSLv3_method();
        tcpConnection->ctx = SSL_CTX_new(method);
        err = ERR_get_error();
        if (err != 0)
        {
            printf("ERR_get_error = %lu\n", err);
            efunc = ERR_func_error_string(err);
            printf("err func: %s\n", efunc);
            elib = ERR_lib_error_string(err);
            printf("err lib: %s\n", elib);
            ereason = ERR_reason_error_string(err);
            printf("err reason: %s\n", ereason);
        }

        if (tcpConnection->ctx == NULL)
        {
            printf("SSL_CTX_new failed\n");
            Closefd(tcpConnection->fd);
            free(tcpConnection);
            return NULL;
        }

        SSL_CTX_set_options(tcpConnection->ctx, SSL_OP_ALL);

        tcpConnection->ssl = SSL_new(tcpConnection->ctx);

        if (tcpConnection->ssl == NULL)
        {
            printf("SSL_new failed\n");
            SSL_CTX_free(tcpConnection->ctx);
            Closefd(tcpConnection->fd);
            free(tcpConnection);
            return NULL;
        }

        if (SSL_set_fd(tcpConnection->ssl, tcpConnection->fd) < 1)
        {
            printf("SSL_set_fd failed\n");
            SSL_free(tcpConnection->ssl);
            SSL_CTX_free(tcpConnection->ctx);
            Closefd(tcpConnection->fd);
            free(tcpConnection);
            return NULL;
        }
        do
        {
            // SSL_WANT_READ errors are normal, just try again if it happens
            connection_status = SSL_connect(tcpConnection->ssl);
        } while (SSL_get_error(tcpConnection->ssl, connection_status) == SSL_ERROR_WANT_READ);

        if (connection_status < 0)
        {
            printf("SSL_connect failed\n");
            SSL_free(tcpConnection->ssl);
            SSL_CTX_free(tcpConnection->ctx);
            Closefd(tcpConnection->fd);
            free(tcpConnection);
            return NULL;
        }
    }
    if(NULL != address)
        free(address);
    return tcpConnection;
}

void TcpDisconnect(TcpConnection *connection, int isHttps)
{
    if (isHttps)
    {
        SSL_shutdown(connection->ssl);
        SSL_free(connection->ssl);
        SSL_CTX_free(connection->ctx);
    }

    Closefd(connection->fd);
    free(connection);
}

int TcpSendSocket(TcpConnection *connection, int isHttps, const char *msg)
{
    int length = 0;

    int data_len = strlen(msg);
    const char *p = msg;
    int iRetOpt = 0, error = 0;
    socklen_t len = 0;
    while (data_len > 0)
    {
        if (isHttps)
        {
            length = SSL_write(connection->ssl, p, data_len);
        }
        else
        {
            length = send(connection->fd, p, data_len, 0);
            if(length == -1)
            {
                LOG_ERR("%s",strerror(errno));
            }
        }
        p += length;
        data_len -= length;

    }

    return length;
}

int TcpReceiveSocket(TcpConnection *connection, int isHttps, char *buffer, int *bufferLength)
{
    TcpResponse *response;
    int errorCode;

    errorCode = TcpParseResponse(connection, isHttps, &response);
    if(errorCode == -1)
    {
        LOG_ERR("%s", "error in TcpParseResponse function\n");
    }
    errorCode = TcpVarifyResponse(errorCode, response);
    if(errorCode == -1)
    {
        LOG_ERR("%s", "error in TcpVarifyResponse function\n");
    }
    if (errorCode > 0)
    {
        TcpHeader *header = response->header;
        const char *contentLengthStr = "Content-Length";

        if (header != NULL && header->value != NULL)
        {
            while (header->next != NULL)
            {
                if (strcmp(header->name, contentLengthStr) == 0)
                    *bufferLength = atoi(header->value);
                header = header->next;
            }
            //if(strstr(header->value,"KeepSession")!=NULL)
            //    printf("llllllllllllllll:\t%s\n",header->value);

            memcpy(buffer, header->value, *bufferLength);
            buffer[*bufferLength] = '\0';
        }
    }

    TcpDestroyResponse(response);

    return errorCode;
}
