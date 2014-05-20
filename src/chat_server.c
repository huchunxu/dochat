/*==============================================================
 *     FileName: chat_server.c
 *         Desc: 服务器功能实现
 *       Author: Hu Chunxu
 *        Email: huchunxu@hust.edu.cn
 *      Version: 0.0.1
 *   LastChange: 2014-05-05 09:57:50
 *      History:
 *=============================================================*/
#include "chat_server.h"

/**
 * @brief 服务器运行主函数
 *
 * @param argc    输入参数的数量
 * @param argv[]  输入参数的值
 *
 * @return 无
 */
int main(int argc, char* argv[])
{
    ChatServer *serv = (ChatServer *) malloc(sizeof(ChatServer)); //服务器分配资源
    if (serv == NULL)
    {
        printf("No memorry for server!\n");
        return -1;
    }

    if (server_init(serv) < 0)     //服务器初始化
    {
        printf("Server initialise faild!\n");
        return -1;
    }

    if (server_run(serv) < 0)           //服务器运行
    {
        printf("Server running faild!\n");
    }

    return 0;
}

/**
 * @brief 服务器端初始化
 *
 * @param serv 服务器端信息结构体
 *
 * @return 
 */
static int server_init(ChatServer *serv)
{
    struct sockaddr_in serv_addr;
    bzero(serv, sizeof(ChatServer));

    serv->port = DEF_PORT;   //设置端口号
    serv->sktfd = socket(AF_INET, SOCK_STREAM, 0); //创建socket
    if (serv->sktfd < 0)
    {
        printf("Server socket error!\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;            //socket类型
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //允许任意地址接入
    serv_addr.sin_port = htons(serv->port);  //端口

    //绑定
    if (bind(serv->sktfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
            < 0)
    {
        printf("Server bind error!\n");
        close(serv->sktfd);
        return -1;
    }

    //创建连接队列
    if (listen(serv->sktfd, MAXWAIT) < 0)
    {
        printf("Server listen error!\n");
        close(serv->sktfd);
        return -1;
    }

    pthread_mutex_init(&(serv->lock), NULL);  //初始化互斥锁
    return 0;
}

/**
 * @brief 服务器运行，处理客户端连接请求
 *
 * @param serv 服务器信息结构体
 *
 * @return 
 */
static int server_run(ChatServer *serv)
{
    serv->epfd = epoll_create(MAXEVENTS);  //创建监听文件

    while (1)
    {
        ChatClient *cli = (ChatClient *) malloc(sizeof(ChatClient));  //分配客户端资源
        bzero(cli, sizeof(ChatClient));

        cli->epfd = serv->epfd;   //监听的文件描述符集合
        cli->sktfd = accept(serv->sktfd, (struct sockaddr *) &(cli->sktaddr),
                &(cli->sktlen));  //等待客户连接，并且返回连接后新建的套接字
        if (cli->sktfd < 0)
        {
            free(cli);
            continue;
        }
        printf("New client connected\n");

        if (chat_add_events(cli, EPOLLIN) < 0)  //添加客户连接到监听文件描述集合，可读监听
        {
            close(cli->sktfd);
            free(cli);
            continue;
        }

        server_add_client(serv, cli);          //添加客户到链表
        if (serv->pid == 0)
        {
            pthread_create(&(serv->pid), NULL, server_event_thread, serv); //创建一个线程，监听是否有客户活动
        }
    }
    pthread_cancel(serv->pid);   //取消进程

    return 0;
}

/**
 * @brief 将客户端信息添加到服务器端队列当中，并至于队首
 *
 * @param serv 服务器端信息结构体
 * @param cli  客户端信息结构体
 */
static int server_add_client(ChatServer *serv, ChatClient *cli)
{
    pthread_mutex_lock(&(serv->lock));
    ChatList *node = (ChatList *) malloc(sizeof(ChatList));
    node->data = cli;
    node->next = serv->cli_list;
    node->prev = NULL;
    if (serv->cli_list)
    {
        serv->cli_list->prev = node;  //保证每个节点在队列的最前边
    }
    serv->cli_list = node;
    pthread_mutex_unlock(&(serv->lock));

    return 0;
}

/**
 * @brief 监听socket的线程
 *
 * @param data 服务器数据
 *
 * @return 
 */
static void *server_event_thread(void *data)
{
    ChatServer *serv = data;
    struct epoll_event evts[MAXEVENTS];

    while (1)
    {
        int fds = epoll_wait(serv->epfd, evts, MAXEVENTS, 100);
        int i = 0;
        for (i = 0; i < fds; i++)
        {
            ChatClient *cli = evts[i].data.ptr;
            int evt = (cli->events & evts[i].events);

            if (evt & EPOLLIN)
            {
                server_get_msg(serv, cli);   //接受数据
            }
            else if (evt & EPOLLOUT)
            {
                server_send_msg(serv, cli);  //发送数据
            }
        }
    }
    return NULL;
}

/**
 * @brief 服务器接收数据
 *
 * @param serv 服务器
 * @param cli  客户端
 *
 * @return 
 */
static int server_get_msg(ChatServer *serv, ChatClient *cli)
{
    char buf[MAXLEN] = { 0 };

    if (socket_readline(cli->sktfd, buf) < 0)
    {
        printf(" read socket error \n");
        server_remove_client(serv, cli);
        return -1;
    }
    char *str = str_strip(buf);
    if (str == NULL)
    {
        return 0;
    }
    if (client_parse_head(cli, buf) < 0)
    {
        printf(" failed to parse client message\n");
        packet_free(cli->pktget);
        cli->pktsnd = NULL;
        return -1;
    }
    if (cli->pktstat != PKT_RDFULL)
    {
        return 0;
    }
    cli->pktstat = PKT_IDLE;

    if (strcmp(cli->pktget->to, SERV_NAME) == 0)
    {
        return server_packet_handler(serv, cli);
    }
    ChatClient *tocli = server_find_client(serv, cli->pktget->to);
    if (tocli == NULL)
    {
        return -1;
    }
    tocli->pktsnd = cli->pktget;
    cli->pktget = NULL;
    chat_add_events(tocli, EPOLLOUT);
    return 0;
}

/**
 * @brief 服务器端删除客户端信息
 *
 * @param serv 服务器数据结构
 * @param cli  客户端数据结构
 *
 * @return 
 */
static int server_remove_client(ChatServer *serv, ChatClient *cli)
{
    pthread_mutex_lock(&(serv->lock));
    ChatList *node = serv->cli_list;
    chat_remove_events(cli);
    while (node && node->data != cli)
    {
        node = node->next;
    }
    if (node == NULL)
    {
        pthread_mutex_unlock(&(serv->lock));
        return -1;
    }

    if (node == serv->cli_list)
    {
        serv->cli_list = node->next;
        if (serv->cli_list)
        {
            serv->cli_list->prev = NULL;
        }
    }

    if (node->next)
    {
        node->next->prev = node->prev;
    }
    if (node->prev)
    {
        node->prev->next = node->next;
    }

    free(node);
    free(cli->name);
    free(cli);

    pthread_mutex_unlock(&(serv->lock));
    return 0;
}

/**
 * @brief 服务器发送数据到客户端
 *
 * @param serv 服务器端数据结构
 * @param cli  客户端数据结构
 *
 * @return 
 */
static int server_send_msg(ChatServer *serv, ChatClient *cli)
{
    client_flush(cli);
    chat_remove_events(cli);
    chat_add_events(cli, EPOLLIN);
    return 0;
}

/**
 * @brief 服务器处理客户端发送的命令
 *
 * @param serv 服务器端数据结构
 * @param cli  客户端数据结构
 *
 * @return 
 */
static int server_packet_handler(ChatServer *serv, ChatClient *cli)
{
    int ret = 0;
    int user_id = 0;
    MsgType msg_type = MSG_TEXT_SEND;

    ChatPacket *pkt = cli->pktget;
    if (pkt == NULL)
    {
        return -1;
    }
    if (pkt->nmsg <= 0)
    {
        ret = -1;
        goto FAILED;
    }
    char *msg = pkt->msg[0];
    ServerCmd cmd = server_parse_cmd(msg);
    switch (cmd)
    {
        case CMD_LOGIN:
            {
                if (pkt->from == NULL)
                {
                    ret = -1;
                    goto FAILED;
                }
                cli->name = strdup(pkt->from);     //获取用户名
                cli->password = pkt->msg[1];       //获取密码
                char buf[MAXLEN] ={0};
               
                if((user_id = check_user(cli->name, cli->password)) > 0)
                {
                    printf("Check success. The user id is %d\n", user_id);
                    msg_type = MSG_LOGIN;
                    send_to_client(cli, buf, msg_type);
                    sprintf(buf, "hello %s! you can use 'help' now!\n", cli->name);
                    printf(" client %s login\n", cli->name);
                }
                else if(user_id == ERR_PWD)
                {
                    printf("The password is error!\n");
                    msg_type = MSG_LOGOUT_ERR_PSW;
                    server_remove_client(serv, cli);
                }
                else if(user_id == ERR_USER)
                {
                    printf("The user %s is not exist!\n", cli->name);
                    msg_type = MSG_LOGOUT_ERR_USER;
                    server_remove_client(serv, cli);
                }
                else
                {
                    printf("The user name %s or password is error! Error id: %d\n",cli->name, user_id);
                    msg_type = MSG_LOGOUT;
                    server_remove_client(serv, cli);
                }

                send_to_client(cli, buf, msg_type);
                break;
            }
        case CMD_REGISTER:
            {
                if (pkt->from == NULL)
                {
                    ret = -1;
                    goto FAILED;
                }
                cli->name = strdup(pkt->from);     //获取用户名
                cli->password = pkt->msg[1];       //获取密码
                char buf[MAXLEN] ={ 0 };
                
                if((user_id = register_user(cli->name, cli->password)) > 0)
                {
                    msg_type = MSG_LOGIN;
                    send_to_client(cli, buf, MSG_LOGIN);
                    printf("Regiser user! name: %s, password: %s\n", cli->name, cli->password);
                    sprintf(buf, "hello %s! The user register successful! you can use 'help' now!\n", cli->name);
                    printf(" client %s login\n", cli->name);
                }
                else if (user_id == ERR_USER)
                {
                    printf("The user %s is exist!\n", cli->name);
                    msg_type = MSG_LOGOUT_ERR_USER_EXIST;
                    server_remove_client(serv, cli);
                }
                else
                {
                    printf("The user name %s or password is error! Error id: %d\n",cli->name, user_id);
                    msg_type = MSG_LOGOUT;
                    server_remove_client(serv, cli);
                }
               
                send_to_client(cli, buf, msg_type);
                
                break;
            }
        case CMD_WHOISON:
            {
                printf(" client %s check who is online\n", cli->name);
                ChatPacket *pktsnd = packet_new(SERV_NAME, cli->name);
                pktsnd->time = gettime();
                pktsnd->type = get_msg_type(MSG_TEXT_SEND);
                ChatList *node = serv->cli_list;
                while (node && node->data)
                {
                    ChatClient *client = node->data;
                    if (client != cli)
                    {
                        packet_add_msg(pktsnd, client->name);
                    }
                    node = node->next;
                }
                cli->pktsnd = pktsnd;
                client_flush(cli);
                break;
            }
        case CMD_LOGOUT:
            printf(" client %s logout\n", cli->name);
            server_remove_client(serv, cli);
            break;
        default:
            break;
    }

FAILED: packet_free(pkt);
        cli->pktget = NULL;
        return ret;
}

/**
 * @brief 在客户队列中查找相应名字的客户
 *
 * @param serv  服务器数据结构
 * @param name  要查找的客户名字
 *
 * @return 
 */
static ChatClient *server_find_client(ChatServer *serv, const char *name)
{
    ChatList *node = serv->cli_list;
    while (node)
    {
        ChatClient *cli = node->data;
        if (strcmp(cli->name, name) == 0)
        {
            return cli;
        }
        node = node->next;
    }

    return NULL;
}

