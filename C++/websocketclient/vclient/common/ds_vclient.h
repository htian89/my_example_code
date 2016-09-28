#ifndef __DS_VCLIENT_H__
#define __DS_VCLIENT_H__

#define BUFFERLENGTH 20000
#define MAX_NUM_APPLICATION 128
#define MAX_NUM_VIRTUALDISK 10

#define MAX_LEN	512
#define MIN_LEN 64

enum SUPPORT_PROTOCAL{NO_SUPPORT=-1, BOTH_SUPPORT, ONLY_RDP, ONLY_FAP, ONLY_TERMINAL};  //-1:No support both protocals. 0:Support both protocals
                                                                //1:Only support rdp protocal 2: only support fap protocal 3: only terminal

enum desktoptype{VIRTUALAPP=0, NORMALDESKTOP, DESKTOPPOOL, REMOTEDESKTOP, TERMINAL};

struct FileInfo{
    char filePath[1024];
    char size[MIN_LEN];
    char type[MIN_LEN];
};

struct FileInfoList{
    struct FileInfo stFirstPath;
    struct FileInfo stAlterNatePath;
    struct FileInfo stPresentPath;
};
typedef struct FileInfoList FILEINFOLIST;
struct NetworkSet
{
    char serverAddress[MAX_LEN];
    char port[MIN_LEN];
    int  isHttps;
};

struct Network
{
    struct NetworkSet stFirstServer;
    struct NetworkSet stAlternateServer;
    struct NetworkSet stPresentServer;
////----this is the old structure
//    char firstServer[MAX_LEN];
//    char alternateServer[MAX_LEN];
//    char presentServer[MAX_LEN];
//    char port[MIN_LEN];
//    int isHttps;
};
typedef struct Network NETWORK;

struct Userinfo
{
    char domain[MAX_LEN];
    char username[MAX_LEN];
    char password[MAX_LEN];
    char newpassword[MAX_LEN];
    //for italc
    char ip[MAX_LEN];
    char role[MAX_LEN];
    char seatnumber[512];
    //for vclass login
    char uuid[512];
};
typedef struct Userinfo USER_INFO;

struct NtAccountInfo  //NtAccount
{
    char logonTicket[MAX_LEN];
    char ntUsername[MAX_LEN];
    char ntPassword[MAX_LEN];
    char ntDomain[MAX_LEN];
    char broadcastscreen_ip[MIN_LEN];
    unsigned short broadcastscreen_port;
    int Role;
};
typedef struct NtAccountInfo NT_ACCOUNT_INFO;

struct ResourceInfo
{
    char ipAddr[MAX_LEN];
    char port[MIN_LEN];
    char Uuid[MAX_LEN]; // post LocalMac to server, so get Uuid from server and pass the Uuid to fap
    char usb_uuid[MAX_LEN]; //for tcp-usbip
    char SecurityUuid[MAX_LEN]; // the Uuid for fap Security desktop
    char resourceTicket[MAX_LEN];
    char SecurtityToken[MAX_LEN];//added by qinchuan@FAP, base64 encoded
    char SecurtityPort[MAX_LEN];  // added by qinchuan@FAP
    int iIdleTime; //default set to -1
};
typedef struct ResourceInfo RESOURCE_INFO;

struct RailInfo
{
    char applicationName[MAX_LEN];
    char workingDirectory[MAX_LEN];
    char arguments[MAX_LEN];
};
typedef struct RailInfo RAIL_INFO;

struct ResourceParameters
{
    char hostName[MAX_LEN];
    char hostPort[MAX_LEN];
    char forceUsername[MAX_LEN];
    char forcePassword[MAX_LEN];
    char forceDomain[MAX_LEN];
    char alternateShell[MAX_LEN];
    char workingDir[MAX_LEN];
    int protocol;
    char colorDepth[MIN_LEN];
    char fullScreen[MIN_LEN];
    char resolution[MIN_LEN];
    int compression;
    int performance;
    RAIL_INFO rail;
    int audio;
    int audioIn;
    int printer;
    int disk;
    int smartcard;
    int serialPort;
    int parallelPort;
    int clipboard;
    char usb[MAX_LEN];
    char usbType[10][MIN_LEN];
};
typedef struct ResourceParameters RESOURCE_PARAMETERS;

struct AppList
{
    enum desktoptype desktopType;
//desktoptool
    char uuid[MAX_LEN];
    char name[MAX_LEN];
    int displayprotocol;
    int type;
    int userAssignment;
    int sourceType;
    int enable;
    int vmState;
    int rdpServiceState;
    int state;
    int powerOnVmNum;
    int rdpOnVmNum;
    char description[MAX_LEN];
    char OsType[MAX_LEN];
///app
    char hostname[MAX_LEN];
    char hostDescription[MAX_LEN];
//remote...
//terminal
    char TerminalName[MAX_LEN];
    char TerminalIp[MAX_LEN];
    RESOURCE_PARAMETERS resParams;
};
typedef struct AppList APP_LIST;


struct SerialParallelPort
{
    int serialLocal, serialRemote;
    int parallelLocal, parallelRemote;
};
typedef struct SerialParallelPort SERIAL_PARALLEL_PORT;

struct VirtualDisk
{
    char devicePath[MAX_LEN];
    char diskSize[MAX_LEN];
    char sizeUnit[MAX_LEN];
};
typedef struct VirtualDisk VIRTUALDISK;

struct FloatingBar
{
    int rdp;
    int spice;
};
typedef struct FloatingBar FLOATING_BAR;

struct Rdp_Params
{
    char hostName[MAX_LEN];
    char hostPort[MAX_LEN];
    char forceUsername[MAX_LEN];
    char forcePassword[MAX_LEN];
    char forceDomain[MAX_LEN];
    char alternateShell[MAX_LEN];
    char workingDir[MAX_LEN];
    RAIL_INFO rail;
    char colorDepth[MIN_LEN];
    char fullScreen[MIN_LEN];
    char resolution[MIN_LEN];
    int compression;
    int performance;
    int audio;
    int audioIn;
    int printer;
    int disk;
    int smartcard;
    int serialPort;
    int parallelPort;
    int clipboard;
    char usb[MAX_LEN];
    SERIAL_PARALLEL_PORT port;
};
typedef struct Rdp_Params RDP_PARAMS;

struct SelectApplication
{
    int protocol;
    int bypassProtocol;
    char bypassIP[MAX_LEN];
    char bypassPort[MAX_LEN];
    RDP_PARAMS rdpParams;
    char ticket[MAX_LEN];
};
typedef struct SelectApplication SELECTAPPLICATION;





#endif // DS_VCLIENT_H
