#include <QFile>
#include <QDebug>
#include <QTextStream>
#include "rh_ifcfg.h"
#include "filepath.h"
#include "log.h"
#include <sys/socket.h>
#include <net/if.h>		/* for ifconf */
#include <linux/sockios.h>	/* for net status mask */
#include <netinet/in.h>		/* for sockaddr_in */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <fcntl.h>
#include <string.h>


//debug
#include <iostream>
using namespace std;

#define MAX_INTERFACE	(16)

struct route_info
{
    struct in_addr dstAddr;
    struct in_addr srcAddr;
    struct in_addr gateWay;
    char ifName[IF_NAMESIZE];
};



int readNlSock(int sockFd, char *bufPtr, size_t buf_size, int seqNum, int pId)
{
    struct nlmsghdr *nlHdr;
    int readLen = 0, msgLen = 0;

    do
    {
        /* Recieve response from the kernel */
        if((readLen = recv(sockFd, bufPtr, buf_size - msgLen, 0)) < 0)
        {
            perror("SOCK READ: ");
            return -1;
        }

        nlHdr = (struct nlmsghdr *)bufPtr;

        /* Check if the header is valid */
        if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
        {
            perror("Error in recieved packet");
            return -1;
        }

        /* Check if the its the last message */
        if(nlHdr->nlmsg_type == NLMSG_DONE)
        {
            break;
        }
        else
        {
            /* Else move the pointer to buffer appropriately */
            bufPtr += readLen;
            msgLen += readLen;
        }

        /* Check if its a multi part message */
        if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
        {
            /* return if its not */
            break;
        }
    }
    while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

    return msgLen;
}

/* parse the route info returned */
int parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)
{
    struct rtmsg *rtMsg;
    struct rtattr *rtAttr;
    int rtLen;

    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

    /* If the route is not for AF_INET or does not belong to main routing table then return. */
    if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
        return -1;

    /* get the rtattr field */
    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
    rtLen = RTM_PAYLOAD(nlHdr);

    for(; RTA_OK(rtAttr,rtLen); rtAttr = RTA_NEXT(rtAttr,rtLen))
    {
        switch(rtAttr->rta_type)
        {
        case RTA_OIF:
            if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
            break;

        case RTA_GATEWAY:
            memcpy(&rtInfo->gateWay, RTA_DATA(rtAttr), sizeof(rtInfo->gateWay));
            break;

        case RTA_PREFSRC:
            memcpy(&rtInfo->srcAddr, RTA_DATA(rtAttr), sizeof(rtInfo->srcAddr));
            break;

        case RTA_DST:
            memcpy(&rtInfo->dstAddr, RTA_DATA(rtAttr), sizeof(rtInfo->dstAddr));
            break;
        }
    }

    return 0;
}

// meat
int get_gatewayip(char *gatewayip, socklen_t size)
{
    int found_gatewayip = 0;

    struct nlmsghdr *nlMsg;
    struct rtmsg *rtMsg;
    struct route_info route_info;
    char msgBuf[8192]; // pretty large buffer

    int sock, len, msgSeq = 0;

    /* Create Socket */
    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
    {
        perror("Socket Creation: ");
        return(-1);
    }

    /* Initialize the buffer */
    memset(msgBuf, 0, sizeof(msgBuf));

    /* point the header and the msg structure pointers into the buffer */
    nlMsg = (struct nlmsghdr *)msgBuf;
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);

    /* Fill in the nlmsg header*/
    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
    nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .

    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.
    nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.

    /* Send the request */
    if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
    {
        fprintf(stderr, "Write To Socket Failed...\n");
        return -1;
    }

    /* Read the response */
    if((len = readNlSock(sock, msgBuf, sizeof(msgBuf), msgSeq, getpid())) < 0)
    {
        fprintf(stderr, "Read From Socket Failed...\n");
        return -1;
    }

    /* Parse and print the response */
    for(; NLMSG_OK(nlMsg,len); nlMsg = NLMSG_NEXT(nlMsg,len))
    {
        memset(&route_info, 0, sizeof(route_info));
        if ( parseRoutes(nlMsg, &route_info) < 0 )
            continue;  // don't check route_info if it has not been set up

        // Check if default gateway
        if (strstr((char *)inet_ntoa(route_info.dstAddr), "0.0.0.0"))
        {
            // copy it over
            inet_ntop(AF_INET, &route_info.gateWay, gatewayip, size);
            found_gatewayip = 1;
            break;
        }
    }

    ::close(sock);

    return found_gatewayip;
}


rh_ifcfg::rh_ifcfg()
{
    memset(m_netPortName,0, sizeof(m_netPortName));
    memset(&m_ipInfo, 0, sizeof(m_ipInfo));
    initIfcfgPath();
//    m_ifcfg_filepath = "/etc/sysconfig/network-scripts/ifcfg-eth0";
}

void rh_ifcfg::initIfcfgPath()
{
    do{
        int fd;
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(fd == -1){
            LOG_ERR("socket failed; %d", errno);
            break;
        }
        if(fd > 0){
           get_if_info(fd);
           close(fd);
        }
        get_gatewayip(m_ipInfo.gateway, sizeof(m_ipInfo.gateway));
    }while(0);
}

int rh_ifcfg::get_if_info(int fd)
{
    struct ifconf ifc;
    struct ifreq buf[MAX_INTERFACE];
    memset(buf,0, sizeof(struct ifreq)*MAX_INTERFACE);

    int if_num = 0;
    int ret = 0;

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)(buf);
    // 获取网口列表信息 ifc
    ret = ioctl(fd, SIOCGIFCONF, (char*)(&ifc));
    if(ret == -1){
        LOG_ERR("get net interface list failed %d", errno);
        return -1;
    }
    if_num = ifc.ifc_len/sizeof(struct ifreq);

    while(if_num-- > 0){
        //网口名字
        cout << buf[if_num].ifr_name << endl;
        ret = ioctl(fd, SIOCGIFFLAGS, (char*)(&buf[if_num]));
        if(ret == -1){
            LOG_ERR("get %s infomation failed", buf[if_num].ifr_name);
        }
        //获取网口状态
        short int flags = buf[if_num].ifr_flags;
        if(flags & IFF_UP){//网口已开启
             LOG_INFO("IFF_UP");
        }
        if(flags & IFF_BROADCAST){//广播地址有效
            LOG_INFO("IFF_BROADCAST");
        }
        if(flags &IFF_LOOPBACK){ //回环地址
            LOG_INFO("IFF_LOOPBACK");
        }
        if( flags & IFF_MULTICAST){ //支持多播
            LOG_INFO("IFF_MULTICAST");
        }
        if(flags& IFF_RUNNING){  // IFF_UP && IFRUNNING
            LOG_INFO("IFF_RUNNING");
        }

        if((flags & IFF_UP) && (flags & IFF_RUNNING) &&(!(flags & IFF_LOOPBACK))){
            m_ifcfg_filepath = "/etc/sysconfig/network-scripts/ifcfg-";
            m_ifcfg_filepath += buf[if_num].ifr_name;
            strcpy(m_ipInfo.name,buf[if_num].ifr_name);
            ret = ioctl(fd, SIOCGIFADDR, (char*)(&buf[if_num]));
            if(ret == -1){
                LOG_ERR("get %s ipaddr failed", buf[if_num].ifr_name);
            }else{
                strcpy(m_ipInfo.ip,inet_ntoa(((struct sockaddr_in *)(&(buf[if_num].ifr_addr)))->sin_addr));
            }
            ret = ioctl(fd, SIOCGIFNETMASK, (char*)(&buf[if_num]));
            if(ret == -1){
                 LOG_ERR("get %s netmask failed", buf[if_num].ifr_name);
            }else{
                strcpy(m_ipInfo.netmask, inet_ntoa(((struct sockaddr_in *)(&(buf[if_num].ifr_netmask)))->sin_addr));
            }
            FILE *file;
            char buf[256];
            char dns[256];
            bzero(buf, sizeof(buf));
            bzero(dns, sizeof(dns));
            file = fopen("/etc/resolv.conf", "r");
            if( NULL == file){
                LOG_ERR("fopen /etc/resolv.conf error");
            }else{
                while(fgets(buf, sizeof(buf), file) != NULL && !feof(file)){
                    if( strstr(buf, "#") != NULL){
                        continue;
                    }else{
                        if( strstr(buf, "nameserver") != NULL){
                            strcpy(dns, strstr(buf, "nameserver"));
                            strcpy(m_ipInfo.dns1, dns + strlen("nameserver "));
                            if(strstr(m_ipInfo.dns1, "\n") != NULL){
                                m_ipInfo.dns1[strlen(m_ipInfo.dns1)-1] = '\0';
                            }
                            printf("dns1: %d\n",strlen(m_ipInfo.dns1));
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
    return 0;
}

rh_ifcfg::~rh_ifcfg()
{

}

// input key, ouput value
bool rh_ifcfg::get_value(QString key, QString *value)
{
    QFile data(m_ifcfg_filepath);
    if (data.open(QFile::ReadOnly )) {
        QTextStream in(&data);

        // reading
        QString line;
        do {
            line = in.readLine();
            qDebug() << "rh_ifcfg line: " << line << endl;
            if (line.startsWith(key)) {
                //store the string after '=' to QString value
                *value = line.section('=', 1, 1);
                qDebug() << "rh_ifcfg value got: " << *value << endl;
                break;
            }
        } while (!line.isNull());

        data.close();
        return true;
    } else {
        // error processing
        return false;
    }
}

// input key, value , write them to the ifcfg_eth0 file
bool rh_ifcfg::set_value(QString key, QString value)
{
    if(value.isEmpty()){
        LOG_ERR("%s", "key value is empty");
        return false;
    }
    QFile ifcfg(m_ifcfg_filepath);
    QFile tmp(userPath + "rh_ifcfg_tmp");
    QString line;
    bool has_key = false; //flag to show whether certain key-value pair exists, for example 'dns1=...'

    // open file for reading
    if (ifcfg.open(QFile::ReadOnly ) && tmp.open(QFile::WriteOnly)) {
        QTextStream in(&ifcfg);
        QTextStream out(&tmp);
        do {
            line = in.readLine();
            qDebug() << "rh_ifcfg line, reading: " << line << endl;
            if (line.startsWith(key)) {
                out << line.section('=', 0, 0) << "=" << value << endl;
                has_key = true;
            } else if (!line.isEmpty()){
                out << line << endl;
            } else {
                continue;
            }
        } while (!line.isNull());

        if (has_key == false) {
            out << key << "=" << value << endl;
        }

        ifcfg.close();
        tmp.close();

        // now, the rh_ifcfg_tmp file is what we want, we then replace ifcfg_eth0 with it
        if (ifcfg.open(QFile::WriteOnly) && tmp.open(QFile::ReadOnly)) {
            QTextStream in(&tmp);
            QTextStream out(&ifcfg);
            do {
                line = in.readLine();
                qDebug() << "rh_ifcfg_line, writing: " << line << endl;
                out << line << endl;
            } while (!line.isNull());

            ifcfg.close();
            tmp.close();
            system("sync");
        }
        return true;
    } else {
        // error processing
        return false;
    }
}
