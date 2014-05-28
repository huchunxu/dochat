#ifndef _CHAT_H_
#define _CHAT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAXLINK 100
#define MAXMSG 100       //最大信息数量
#define MAXLEN 1024
#define MAXWAIT 50       //未处理连接的最大数目
#define MAXEVENTS 1000   //最大的连接数
#define MAXTALK 10

#define SERV_NAME "SERVER"  //服务器名字

#define SAFE_FREE(mem) \
do{ \
    if(mem){ \
        free(mem); \
        mem = NULL; \
    } \
}while(0)

typedef enum{
    HEAD_START = 0,
    HEAD_TO,
    HEAD_FROM,
    HEAD_TYPE,
    HEAD_MSG,
    HEAD_TIME,
    HEAD_FDBK,
    HEAD_END,
    HEAD_LAST,
}HeadType;

//发送的消息类型
typedef enum{
    MSG_LOGIN,
    MSG_LOGOUT,
    MSG_LOGOUT_ERR_PSW,
    MSG_LOGOUT_ERR_USER,
    MSG_LOGOUT_ERR_USER_EXIST,
    MSG_FILE_SEND,
    MSG_TEXT_SEND,
    MSG_USER_ONLINE,
    MSG_USER_OFFLINE,
    MSG_USER_ALL,
    MSG_CHECK_USER_ONLINE,
    MSG_LAST,
}MsgType;


typedef enum{
    CMD_LOGIN,
    CMD_REGISTER,
    CMD_WHOISON,
    CMD_SHOWUSER,
    CMD_LOGOUT,
    CMD_LAST,
}ServerCmd;

//数据包的状态
typedef enum{
    PKT_IDLE,
    PKT_READ,
    PKT_WRITE,
    PKT_RDFULL,
    PKT_WRFULL,
    PKT_OTHER,
}PktStatus;

//信息数据包结构
typedef struct{
    char *to;
    char *from;
    char *time;
    int nmsg;
    int curmsg;
    char *type;
    char *msg[MAXMSG];
    char *fdbk;
}ChatPacket;

 //客户端连接队列
typedef struct _ChatList ChatList;   
struct _ChatList{
    void *data;
    ChatList *next;
    ChatList *prev;
};

//客户端信息结构体
typedef struct{
    char *name;
    char *password;
    int status;
    int events;
    int sktfd;
    int epfd;
    int login_stat;
    ChatPacket *pktsnd;
    ChatPacket *pktget;
    PktStatus pktstat;
    socklen_t sktlen;
    struct sockaddr_in sktaddr;
}ChatClient;

//服务器信息结构体
typedef struct{
    int maxwait;
    int port;
    ChatList *cli_list;
    ChatPacket *msg_offline[MAXMSG];
    int nmsg_offline;
    int sktfd;
    int epfd;
    pthread_t pid;
    pthread_mutex_t lock;
}ChatServer;


#endif
