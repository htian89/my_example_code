#ifndef ERRORCODE_H
#define ERRORCODE_H

// SUCCESS
#define ERROR_OK								0

// REQUEST ERROR
#define ERROR_CONNECT_FAIL						-1		/* Can not connect to server */
#define ERROR_FAIL								-101	/* Fail */
#define ERROR_USER_TICKET_INVALID				-102	/* User ticket invalid */
#define ERROR_USER_REMOVED						-103	/* User removed */
#define	ERROR_APP_TICKET_INVALID				-104	/* Application ticket invalid */
#define ERROR_APP_REMOVED						-105	/* Application removed */
#define	ERROR_PARAMS							-106	/* Params wrong */
#define ERROR_REQUEST_URL						-107	/* Remote request url wrong */
#define ERROR_CONNECT_SERVER					-108	/* Can not connect to server */
#define ERROR_NO_AUTHORITY						-109	/* Has no authority */
#define ERROR_OBJECT_NOT_EXIST					-110	/* Object(Desktoppool, Fixdesktop, virtualapplication) not exist*/

// SERVICE ERROR
#define ERROR_LOGIN_PASSWORD					-201	/* User password wrong */
#define ERROR_LOGIN_DOMAIN						-202	/* Domain wrong */
#define ERROR_LICENSE_EXPIRED					-203	/* License expired */
#define ERROR_LICENSE_EXCEED					-204	/* User number up to limit */
#define ERROR_ACL_FAIL							-205	/* Acl check fail */
#define ERROR_ACL_FAIL_IP						-206	/* IP limited */
#define ERROR_UNKNOWN_HOST						-207	/* Unknown host */
#define ERROR_USERNAME_EXIST					-208	/* Username to be registered is existed*/
#define ERROR_PERSONALDISK_NOT_EXIST			-209	/* Personal disk not exist*/
#define ERROR_SUPPORT_TOKEN_LOGIN_ONLY          -210    /* Only support token login */
#define ERROR_SUPPORT_NORMAL_LOGIN_ONLY         -211    /* Only support normal login*/
#define ERRO_GET_LICENCE_INFO_FAILED            -212    /* used in the interface: loginSession*/
#define ERROR_CONNECT_DOMAIN_FAILED             -213
#define ERROR_PROXY_SERVER_CLIENT_ACCESS_DENIED -214    /* current server is a proxy server, client login denied */
// DESKTOP POOL ERROR
#define ERROR_DESKTOPPOOL_UNREACHABLE			-301	/* Desktop pool can not be reached */
//#define ERROR_ASSIGN_NO_DESKTOP					-302	/* No desktop assigned */
#define ERROR_NO_AVAILABLE_DESKTOP_IN_POOL		-302	/* No available desktop resource in the desktop pool */
#define ERROR_DESKTOPPOOL_NOT_EXIST				-303	/* Desktop pool not exist */
#define ERROR_DESKTOPPOOL_STATUS_EXCEPTION		-304	/* Desktop pool status exception */
#define ERROR_DESKTOP_USED_BY_OTHER				-305	/* Pool desktop being used by other */
#define ERROR_DESKTOP_IP_EXCEPTION				-306	/* Pool desktop IP exception */
#define ERROR_DESKTOP_UPDATING					-307	/* Pool desktop is updating */
#define ERROR_ATTACH_DESKTOPPOOLCLOSED			-308	/* Attach fail, desktoopool has closed */
#define ERROR_ATTACH_DESKTOPPOOLUNREACHABLE		-309	/* Attach fail, desktoppool can not be reached */
#define ERROR_ATTACH_DESKTOPPOOLCANNOTCONNECT	-310	/* Attach fail, desktoppool can not connect */
#define ERROR_UNABLE_TO_ATTACH					-311	/* Unable to attach, disk is attaching, unattach fail */
#define ERROR_HAS_NOT_BEEN_ASSIGNED				-313	/* Has not been assigned a desktop */
#define ERROR_DESKTOP_NOT_CONNECTED				-312	/* Not connect to desktop, please attach after connected */
#define ERROR_NOT_SUPPORT_FAP                   -314    /* Not support spicec protocal*/
#define ERROR_HAS_NOT_BEEN_ASSIGNED				-313	/* Has not been assigned a desktop */
#define ERROR_EXCEED_DESPTOPPOOL_MAXIMUMLIMIT   -316    /* Start the desktop pool exceeds the maximum limit */
#define ERROR_NOT_SUPPORT_SELFSERVICE           -317    /* Do not support self-service capabilities */
#define ERROR_DESKTOPPOOL_STOP					-350	/* Desktop pool stopped */
#define ERROR_DESKTOPPOOL_POWERING				-351	/* Desktop pool powering */

#define ERROR_VCENTER_UNREACHABLE				-500	/* vCenter can not be reached*/
#define ERROR_CREATE_CONNECTION_FAIL			-501	/* Create connection fail */
#define ERROR_ANOTER_PROTOCOL_ALIVE				-502	/* Another protocol is being used */
#define ERROR_APPSERVER_NOT_FOUND				-503	/* Can not find application server */
#define ERROR_OPEN_CHANNEL						-504	/* Open channel fail */
#define ERROR_CLOSE_CHANNEL						-505	/* Close channel fail */

// TOKEN AUTH ERROR
#define ERROR_FORBID_BINDING_TOKEN              -400    /* Forbid binding token self*/
#define ERROR_VALID_IDENTIFICATION              -401    /* Failed to identify*/
#define ERROR_INVALID_SEQUENCE_NUMBER			-401	/* Invalid sequence number */
#define ERROR_EMPTY_TOKEN_IMEI					-601	/* Token serial number is empty */
#define ERROR_EMPTY_DYNAMIC_PASSWORD			-602	/* Dynamic password is empty */
#define ERROR_INVALID_TOKEN_TYPE				-603	/* Invalid token type */
#define ERROR_INVALID_TOKEN_SEED				-604	/* Token seed is empty or not long enough */
#define ERROR_TOKEN_EXPIRED						-605	/* Token expired */
#define ERROR_TOKEN_FREEZE						-606	/* Token freeze */
#define ERROR_TOKEN_LOST						-607	/* Toekn report for loss */
#define ERROR_TOKEN_LOKED						-608	/* Token locked */
#define ERROR_TIME_EXCEPTION					-609	/* Time exception */
#define ERROR_FAIL_LOAD_KEY						-610	/* Fail to load key */
#define ERROR_FAIL_LOAD_CONFIGURATION			-611	/* Fail to load token configuration file */
#define ERROR_DYNAMIC_PASSWORD					-612	/* Dymaic password is wrong */
#define ERROR_CALCULATE_EXCEPTION				-613	/* Calculate exception */
#define ERROR_PASSWORD_EXCEED					-614	/* Dynamic password has reached the limited auth numbers */
#define ERROR_FAILED_FIND_DESKTOP_IP            -700    /* Can not find the desktop's ip*/
#define ERROR_RDP_CLOSED                        -701    /* Rdp service is closed*/
#define ERROR_DESKTOP_NOT_LAUNCH                -702    /* Desktop has not been launched*/
#define ERROR_NO_YOUR_DESKTOP_IN_POOL         -703     /* There is no desktop belongs to you in the pool */

#endif // ERRORCODE_H
