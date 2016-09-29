#include <types.h>
#include <ev_epoll.h>
#include <monitor.h>
#include <cJSON.h>

#define MONITOR_NAME_LEN 16

static char *msg_format_error(cJSON *in)
{
    cJSON *out;
    char *out_msg;
    char *action = cJSON_GetObjectItem(in, "Action")->valuestring;
    out = cJSON_CreateObject();
    cJSON_AddItemToObject(out, "Action", cJSON_CreateString(action));
    cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(MSG_FORMAT_ERR));

    out_msg = cJSON_Print(out); 
    printf("%s\n",out_msg); 
    cJSON_Delete(out); 
    return out_msg; 
}

#define GET_INT_FROM_JSON(json, string, value)          \
do {                                                    \
    cJSON *tmp;                                         \
    tmp = cJSON_GetObjectItem(json,string);             \
    if(tmp == NULL){                                    \
        out_msg = msg_format_error(json);               \
        return out_msg;                                 \
    }                                                   \
    value = tmp->valueint;                                \
} while(0)

#define GET_STRING_FROM_JSON(json, string, value)       \
do {                                                    \
    cJSON *tmp;                                         \
    tmp = cJSON_GetObjectItem(json,string);             \
    if(tmp == NULL){                                    \
        out_msg = msg_format_error(json);               \
        return out_msg;                                 \
    }                                                   \
    strcpy(value, tmp->valuestring);                    \
} while(0)

/*
 * print all items
 */
void cJSON_debug(cJSON *json)
{
    char *out = cJSON_Print(json);
    printf("%s\n", out);
    free(out);
}

cJSON *get_one_monitor_info(int n)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "Status", cJSON_CreateNumber(monitor[n].status));
    if(monitor[n].status == USED_MONITOR)
    {
        cJSON_AddItemToObject(json, "Mode", cJSON_CreateNumber(monitor[n].mode));
        cJSON_AddItemToObject(json, "targetIp", cJSON_CreateString(monitor[n].targetIp));
        cJSON_AddItemToObject(json, "targetPort", cJSON_CreateNumber(monitor[n].targetPort)); 
    }
}

static char *monitor_controller_ready(cJSON *in)
{
    cJSON *out;
    char *out_msg;
    out = cJSON_CreateObject();
    cJSON_AddItemToObject(out, "Action", cJSON_CreateString("Ready"));

    out_msg = cJSON_Print(out); 
    printf("%s\n",out_msg); 
    cJSON_Delete(out); 
    return out_msg;
}

static char *get_monitors_info(cJSON *in)
{
    cJSON_debug(in);
    cJSON *out;
    char *out_msg;
    int n;
    char name[MONITOR_NAME_LEN];
    out = cJSON_CreateObject();
    cJSON_AddItemToObject(out, "Action", cJSON_CreateString("GetMonitorsInfo"));
    cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(NO_ERROR));
    cJSON_AddItemToObject(out, "MonitorNum", cJSON_CreateNumber(controller->monitor_num));
    for(n = 0; n < controller->monitor_num; n++)
    {
        cJSON *json = cJSON_CreateObject();
        sprintf(name, "Monitor%d", n+1);
        cJSON_AddItemToObject(json, "Status", cJSON_CreateNumber(monitor[n].status));
        if(monitor[n].status == USED_MONITOR)
        {
            cJSON_AddItemToObject(json, "Mode", cJSON_CreateNumber(monitor[n].mode));
            cJSON_AddItemToObject(json, "targetIp", cJSON_CreateString(monitor[n].targetIp));
            cJSON_AddItemToObject(json, "targetPort", cJSON_CreateNumber(monitor[n].targetPort)); 
        }
        cJSON_AddItemToObject(out, name, json);
    }

    out_msg = cJSON_Print(out); 
    printf("%s\n",out_msg); 
    cJSON_Delete(out); 
    return out_msg; 
}

static char *connect_to_monitor(cJSON *in)
{
    cJSON_debug(in);
    cJSON *out;
    char *out_msg;
    int n;
    GET_INT_FROM_JSON(in, "Monitor", n);

    out = cJSON_CreateObject();
    cJSON_AddItemToObject(out, "Action", cJSON_CreateString("ConnectToMonitor"));
    if(n < 1 || n > controller->monitor_num)
    {
        cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(NO_THIS_MONITOR));
        goto print_out;
    }

    GET_INT_FROM_JSON(in, "Mode", monitor[n-1].mode);
    GET_STRING_FROM_JSON(in, "TargetIp", monitor[n-1].targetIp);
    GET_INT_FROM_JSON(in, "TargetPort", monitor[n-1].targetPort);

    if(monitor[n-1].status == USED_MONITOR && monitor[n-1].pid > 0)
    {
        kill(monitor[n-1].pid, SIGINT);
    }
    use_monitor(n); 

    cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(NO_ERROR));

print_out:
    out_msg = cJSON_Print(out); 
    printf("%s\n",out_msg); 
    cJSON_Delete(out); 
    return out_msg; 
}

static char *disconnect_monitor(cJSON *in)
{
    cJSON_debug(in);
    cJSON *out;
    char *out_msg;
    int n;
    GET_INT_FROM_JSON(in, "Monitor", n);

    out = cJSON_CreateObject();
    cJSON_AddItemToObject(out, "Action", cJSON_CreateString("DisconnectMonitor"));

    if(n < 1 || n > controller->monitor_num)
    {
        cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(NO_THIS_MONITOR));
        goto print_out;
    }

    if(monitor[n-1].status == FREE_MONITOR)
    {
        cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(DISCONNECT_FREE_MONITOR));
        goto print_out;
    }
    if(monitor[n-1].pid > 0)
        kill(monitor[n-1].pid, SIGINT);

    cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(NO_ERROR));

print_out:
    out_msg = cJSON_Print(out); 
    printf("%s\n",out_msg); 
    cJSON_Delete(out); 
    return out_msg; 
}

static char *unsupport_action(char *action)
{
    cJSON *out;
    char *out_msg;
    out = cJSON_CreateObject();
    cJSON_AddItemToObject(out, "Action", cJSON_CreateString(action));
    cJSON_AddItemToObject(out, "ErrorCode", cJSON_CreateNumber(UNSUPPORT_ACTION));

    out_msg = cJSON_Print(out); 
    printf("%s\n",out_msg); 
    cJSON_Delete(out); 
    return out_msg; 
}

int vaccess_handle(int fd, char *in)
{
    cJSON *json;
    char *out;
    char *action;

    json = cJSON_Parse(in);
    if(json == NULL)
    {
        LLOG_ERR("not a json message");
        return -1;
    }
    
    cJSON *item = cJSON_GetObjectItem(json, "Action");
    if(item == NULL)
    {
        LLOG_ERR("cann't find Action item");
        return -1;
    }
    
    action = item->valuestring;
    LLOG("action = %s", action);
    if(strcmp(action, "Ready") == 0) 
    { 
        out = monitor_controller_ready(json); 
        _send(fd, out, strlen(out));  
    } 
    else if(strcmp(action, "GetMonitorsInfo") == 0) 
    { 
        out = get_monitors_info(json); 
        _send(fd, out, strlen(out));  
    } 
    else if(strcmp(action, "ConnectToMonitor") == 0) 
    { 
        out = connect_to_monitor(json); 
        _send(fd, out, strlen(out));  
    } 
    else if(strcmp(action, "DisconnectMonitor") == 0) 
    { 
        out = disconnect_monitor(json); 
        _send(fd, out, strlen(out));  
    } 
    else 
    { 
        LLOG_ERR("unsupport action: %s", action); 
        out = unsupport_action(action); 
        _send(fd, out, strlen(out));  
    }

    free(out);
    cJSON_Delete(json);
    return 0;
}
