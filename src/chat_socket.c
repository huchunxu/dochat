#include "chat_socket.h"

static const char *g_cmd[CMD_LAST] = {
    [CMD_LOGIN] = "LOGIN",
    [CMD_REGISTER] = "REGISTER",
    [CMD_WHOISON] = "WHOISON",
    [CMD_SHOWUSER] = "SHOWUSER",
    [CMD_LOGOUT] = "LOGOUT",
};


int chat_add_events(ChatClient *cli, int events)
{
    struct epoll_event epv;
    cli->events = events;
    epv.events = events;
    epv.data.ptr = cli;

    int op;
    if(cli->status != 0)
    {
        op = EPOLL_CTL_MOD; //修改已经注册的fd的监听事件
    }
    else
    {
        op = EPOLL_CTL_ADD; //注册新的fd到epfd中
    }

    if(epoll_ctl(cli->epfd, op, cli->sktfd, &epv) < 0)
    {
        printf("Failed to set event: %s\n", strerror(errno));
        return -1;
    }

    cli->status = 1;
    return 0;
}

int fd_add_events(int epfd, int fd, int events)
{
    struct epoll_event epv;
    epv.events = events;
    epv.data.fd = fd;

    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epv)<0)
    {
        printf("Failed to add event\n");
        return -1;
    }
    return 0;
}

/**
 * @brief 用户登陆
 *
 * @param cli  客户端数据结构
 * @param pwd  密码
 *
 * @return 
 */
int client_login(ChatClient *cli)
{
    ChatPacket *pkt = packet_new(cli->name, SERV_NAME);
    pkt->nmsg = 2;
    pkt->msg[0] = strdup(g_cmd[CMD_LOGIN]);
    pkt->msg[1] = strdup(cli->password);
    pkt->time = gettime();
    cli->pktsnd = pkt;
    return client_flush(cli);
}

/**
 * @brief 客户端注册账号
 *
 * @param cli    客户端
 * @param user_name 用户名
 * @param pwd 密码
 *
 * @return 
 */
int client_register(ChatClient *cli)
{
    ChatPacket *pkt = packet_new(cli->name, SERV_NAME);
    pkt->nmsg = 2;
    pkt->msg[0] = strdup(g_cmd[CMD_REGISTER]);
    pkt->msg[1] = strdup(cli->password);
    pkt->time = gettime();
    cli->pktsnd = pkt;
    return client_flush(cli);
}

/**
 * @brief 分析客户端输入的数据
 *
 * @param cli     客户端数据结构
 * @param input   输入的数据
 *
 * @return 
 */
int client_parse_input(ChatClient *cli, char *input)
{
    int ret = 0;
    int file_flag = 0;

    input = str_strip(input);    //去掉字符串首尾的空格
    if(input == NULL)
    {
        return -1;
    }

    char *to = NULL;
    char *msg = NULL;
    ServerCmd cmd = CMD_LAST;
    packet_free(cli->pktsnd);  //清空发送数据包
    cli->pktsnd = NULL;

    if(strncmp("to ", input, 3) == 0) //比较前三个字符
    {
        input += 3;
        if(line_parse(input, ':', &to, &msg) == 0)   //以:为分隔符，将字符串分成两段
        {
            cli->pktsnd = packet_new(cli->name, to);
        }
    }
    else if(strncmp("file to ", input, 8) == 0)
    {
        input += 8;
        if(line_parse(input, ':', &to, &msg) == 0)   //以:为分隔符，将字符串分成两段
        {
            cli->pktsnd = packet_new(cli->name, to);
        }
        file_flag = 1;
    }
    else if(strcmp(input, "whoison") == 0)
    {
        cli->pktsnd = packet_new(cli->name, SERV_NAME);
        cmd = CMD_WHOISON;
        msg = (char *) g_cmd[cmd];
    }
    else if(strcmp(input, "showuser") == 0)
    {
        cli->pktsnd = packet_new(cli->name, SERV_NAME);
        cmd = CMD_SHOWUSER;
        msg = (char *) g_cmd[cmd];
    }
    else if(strcmp(input, "help") == 0)
    {
        printf("\rto usr: msg            --------- send msg to 'usr'\n");
        printf("file to usr: filepath  --------- send file to 'usr'\n");
        printf("whoison                --------- check server who is online\n");
        printf("howuser                --------- check all users who are registered\n");
        printf("logout/bye/exit        --------- logout and exit client\n");

        return 0;
    }
    else if(strcmp(input, "logout") == 0 ||
            strcmp(input, "exit")   == 0 ||
            strcmp(input, "bye")    == 0)
    {
        cli->pktsnd = packet_new(cli->name, SERV_NAME);
        cmd = CMD_LOGOUT;
        msg = (char *) g_cmd[cmd];
    }

    if(file_flag)
    {
        cli->pktsnd->type = get_msg_type(MSG_FILE_SEND);    //设置发送的是文件类型
        packet_add_msg(cli->pktsnd, msg);

        FILE *fp = fopen(msg, "r");
        char buffer[MAXLEN];

		if (fp == NULL)
		{
			printf("File: %s Not Found!\n", msg);
		}
        else
        {
            bzero(buffer, MAXLEN);
            while(fgets(buffer, MAXLEN, fp) != NULL)
			{
                packet_add_msg(cli->pktsnd, buffer);
                bzero(buffer, MAXLEN);
			}
			fclose(fp);
			printf("File:\t%s Transfer Finished!\n", msg);
        }
        ret = client_flush(cli);
    }
    if(msg && cli->pktsnd)
    {
        cli->pktsnd->type = get_msg_type(MSG_TEXT_SEND);    //设置发送的是文字类型
        packet_add_msg(cli->pktsnd, msg);
        ret = client_flush(cli);
        if(cmd==CMD_LOGOUT)
        {
            exit(0);
        }
    }
    return ret;
}


/**
 * @brief 按照消息结构发送数据
 *
 * @param cli 客户端数据结构
 *
 * @return 
 */
int client_flush(ChatClient *cli)
{
    int ret = 0;
    ChatPacket *pkt = cli->pktsnd;
    if(pkt == NULL)
    {
        return 0;
    }

    if((ret=socket_send_head(cli->sktfd, HEAD_START, NULL))<0)       goto FAILED;
    if((ret=socket_send_head(cli->sktfd, HEAD_FROM,  pkt->from))<0)  goto FAILED;
    if((ret=socket_send_head(cli->sktfd, HEAD_TO,    pkt->to))<0)    goto FAILED;
    if((ret=socket_send_head(cli->sktfd, HEAD_TIME,  pkt->time))<0)  goto FAILED;
    if((ret=socket_send_head(cli->sktfd, HEAD_FDBK,  pkt->fdbk))<0)  goto FAILED;
    if((ret=socket_send_head(cli->sktfd, HEAD_TYPE,  pkt->type))<0)  goto FAILED;
    
    int i;
    for(i=0;i<(cli->pktsnd->nmsg);i++)
        if((ret=socket_send_head(cli->sktfd, HEAD_MSG, pkt->msg[i]))<0)
            goto FAILED;
    if ((ret=socket_send_head(cli->sktfd, HEAD_END, NULL))<0)
        goto FAILED;
    
FAILED:
    if(ret<0)
    {
        printf("Failed to flush client\n");
    }
    packet_free(pkt);
    cli->pktsnd = NULL;

    return ret;
}


/**
 * @brief 向socket中写一行数据
 *
 * @param sktfd
 * @param line
 *
 * @return 
 */
int socket_writeline(int sktfd, char *line)
{
    int len = strlen(line);
    if(write(sktfd, line, len)<len)   //发送数据
    {
        return -1;
    }

    if(line[len-1] != '\n')            //发送换行标志
    {
        if(write(sktfd, "\n", 1) < 1)
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief 按字节读取socket中的数据
 *
 * @param sktfd
 * @param buf
 *
 * @return 
 */
int socket_readline(int sktfd, char *buf)
{
    int count = 0;
    
    while(1)
    {
        if(read(sktfd, buf+count, 1) < 1)
            break;

        if(buf[count] == '\n')
        {
            buf[count] = '\0';
            return count;
        }
        count++;
    }

    return -1;
}

/**
 * @brief 释放数据包内存
 * 
 * @param pkt 数据包
 *
 * @return 
 */
int packet_free(ChatPacket *pkt)
{
    if(pkt==NULL)
    {
        return -1;
    }

    int i;
    for(i=0;i<pkt->nmsg;i++)
    {
        SAFE_FREE(pkt->msg[i]);
    }
    SAFE_FREE(pkt->from);
    SAFE_FREE(pkt->to);
    SAFE_FREE(pkt->time);
    SAFE_FREE(pkt->fdbk);
    SAFE_FREE(pkt->time);
    SAFE_FREE(pkt);
    return 0;
}

/**
 * @brief 获取当前的时间
 *
 * @return 格式化时间信息
 */
char *gettime(void)
{
    char buf[MAXLEN] = {0};
    time_t timenow = time(0);
    struct tm *tmn = localtime(&timenow);

    sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d", tmn->tm_year+1900,
            tmn->tm_mon+1,tmn->tm_mday,tmn->tm_hour,tmn->tm_min,tmn->tm_sec);

    return strdup(buf);
}

/**
 * @brief 分析收到的数据类型并存储到客户端数据结构中
 *
 * @param cli   客户端数据结构
 * @param head  受到的原始数据
 *
 * @return 
 */
int client_parse_head(ChatClient *cli, char *head)
{
    char *str = NULL;
    str = str_strip(head);
    if(str == NULL)
    {
        return 0;
    }

    char *data = "";
    HeadType type = packet_head_parse(str);
    if(type != HEAD_START && type != HEAD_END)
    {
        data = packet_head_getdata(type, str);
    }

    switch(type)
    {
        case HEAD_START:
            if(cli->pktget)
            {
                printf("Packet is not empty!\n");
                return -1;
            }
            cli->pktstat = PKT_READ;
            cli->pktget = packet_new(NULL, NULL);
            break;
        case HEAD_TO:
            cli->pktget->to = strdup(data);
            break;
        case HEAD_FROM:
            cli->pktget->from = strdup(data);
            break;
        case HEAD_TYPE:
            cli->pktget->type = strdup(data);
            break;
        case HEAD_MSG:
            packet_add_msg(cli->pktget, data);
            break;
        case HEAD_TIME:
            cli->pktget->time = strdup(data);
            break;
        case HEAD_FDBK:
            cli->pktget->fdbk = strdup(data);
            break;
        case HEAD_END:
            //if(cli->pktget->fdbk==NULL && cli->pktget->nmsg==0)
            //if(cli->pktget->fdbk==NULL)
            //{
                //return -1;
            //}

            if(cli->pktget->from == NULL)
            {
                cli->pktget->from = strdup(cli->name);
            }
            if (cli->pktget->time==NULL){
                cli->pktget->time = gettime();
            }
            cli->pktstat = PKT_RDFULL;
            break;
        default:
            break; 
    }

    return 0;
}

int chat_remove_events(ChatClient *cli)
{
    if (cli->status==0)
        return 0;

    struct epoll_event epv;
    epv.events = 0;
    epv.data.ptr = cli;
    
    if (epoll_ctl(cli->epfd, EPOLL_CTL_DEL, cli->sktfd, &epv)<0){
        printf(" failed to remove event\n");
        return -1;
    }

    cli->events = 0;
    cli->status = 0;

    return 0;
}

ServerCmd server_parse_cmd(char *msg)
{
    ServerCmd cmd;
    for (cmd=0; cmd<CMD_LAST; cmd++){
        if(strcmp(msg, g_cmd[cmd])==0){
            return cmd;
        }
    }
    return CMD_LAST;
}

int send_to_client(ChatClient *cli, const char *msg, MsgType msg_type)
{
    ChatPacket *pkt = packet_new(SERV_NAME, cli->name);
    pkt->nmsg = 1;
    pkt->msg[0] = strdup(msg);
    pkt->time = gettime();
    pkt->type = get_msg_type(msg_type);
    cli->pktsnd = pkt;
    return client_flush(cli);
}

