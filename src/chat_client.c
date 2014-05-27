/*==============================================================
 *     FileName: chat_client.c
 *         Desc: 客户端运行主函数
 *       Author: Hu Chunxu
 *        Email: huchunxu@hust.edu.cn
 *      Version: 0.0.1
 *   LastChange: 2014-05-05 16:20:53
 *      History:
 *=============================================================*/
#include "chat_client.h"
#include "interface.h"

static int input_handler(ChatClient *cli);
static int input_parse(ChatClient *cli);
static int socket_handler(ChatClient *cli);

static char inputbuf[MAXLEN] ={ 0 };  //stdin输入缓存
static int inputs = 0;

/**
 * @brief 客户端处理主函数
 *
 * @param argc      输入参数的数量
 * @param argv[]    输入参数值
 *
 * @return 
 */
void client_main(gpointer data)
{
    ChatClient *cli = (ChatClient *)data; 
    struct epoll_event evts[MAXEVENTS];
    while (1)
    {
        int fds = epoll_wait(cli->epfd, evts, MAXEVENTS, 100); //等待终端输入或者socket接收数据
        int i;
        for (i = 0; i < fds; i++)
        {
            int fd = evts[i].data.fd;
            if (fd == fileno(stdin))
            {
                input_handler(cli);     //终端输入处理
            }
            else if (fd == cli->sktfd)
            {
                socket_handler(cli);    //socket接受数据处理
            }
        }
    }
}


/**
 * @brief 输入处理函数
 *
 * @param cli 客户端信息结构体
 *
 * @return 
 */
static int input_handler(ChatClient *cli)
{
    int fd = fileno(stdin);

    if (read(fd, inputbuf + inputs, 1) < 0)  //每次读一个字节
    {
        printf("Stdin input error!\n");
        return -1;
    }

    while (inputbuf[inputs] != '\n')
    {
        inputs++;
        if (read(fd, inputbuf + inputs, 1) < 0)  //每次读一个字节
        {
            printf("Stdin input error!\n");
            return -1;
        }
    }

    if (inputbuf[inputs] == '\n')          //如果输入换行符，则发送数据
    {
        inputbuf[inputs] = '\0';
        return input_parse(cli);
    }
    
    if (inputs == MAXLEN - 1)
    {
        return input_parse(cli);
    }
    return 0;
}

/**
 * @brief 将输入的数据发送
 *
 * @param cli 客户端信息结构体
 *
 * @return 
 */
static int input_parse(ChatClient *cli)
{
    int ret = 0;
    ret = client_parse_input(cli, inputbuf);
    bzero(inputbuf, inputs);
    inputs = 0;
    printf("> ");
    fflush(stdout);

    return ret;
}

/**
 * @brief 读取socket的数据
 *
 * @param cli 客户端信息结构体
 *
 * @return 
 */
static int socket_handler(ChatClient *cli)
{
    char buf[MAXLEN] ={ 0 };

    if (socket_readline(cli->sktfd, buf) < 0)
    {
        printf("Read socket error! maybe the server shutdown!\n");
        exit(0);
    }
    if (client_parse_head(cli, buf) < 0)
    {
        packet_free(cli->pktget);
        cli->pktget = NULL;
        return -1;
    }

    if (cli->pktstat != PKT_RDFULL)
    {
        return 0;
    }

    ChatPacket *pkt = cli->pktget;
    printf("\rFrom: %s\n", pkt->from);
    printf("Time: %s\n", pkt->time);

    int i = 0;
    int msg_type = packet_msg_parse(pkt->type);
    if(!(cli->login_stat))
    {
        switch(msg_type)
        {
            case MSG_LOGOUT:
                printf("> The user name %s or password is error!\n", cli->name);
                fflush(stdout);
                //exit(0);
                gdk_threads_enter();
                login_dialog_create();
                gdk_threads_leave();  
                break;
            case MSG_LOGOUT_ERR_PSW:
                printf("> The password is error!\n");
                fflush(stdout);
                //exit(0);
                gdk_threads_enter();
                login_dialog_create();
                gdk_threads_leave();  
                break;
            case MSG_LOGOUT_ERR_USER:
                printf("> The  user %s is not exist!\n", cli->name);
                fflush(stdout);
                //exit(0);
                gdk_threads_enter();
                login_dialog_create();
                gdk_threads_leave();  
                break;
            case MSG_LOGOUT_ERR_USER_EXIST:
                printf("> The  user %s is exist! Try again!\n", cli->name);
                fflush(stdout);
                //exit(0);
                gdk_threads_enter();
                login_dialog_create();
                gdk_threads_leave();  
                break;
            case MSG_LOGIN:
                cli->login_stat = 1;
                gdk_threads_enter();
                client_parse_input(cli, "whoison");
                main_window_create();
                gtk_widget_show_all(main_window);
                gtk_widget_hide_all(login_window);
                gdk_threads_leave();  
                break;
            default:
                break;
        }
    }

    switch(msg_type)
    {
        case MSG_FILE_SEND:
            {
                FILE *fp = fopen("1.txt", "w");
                if (fp == NULL)
                {
                    printf("File:\t%s Not Found!\n", pkt->msg[0]);
                }

                bzero(buf, MAXLEN);
                for(i=1; i<pkt->nmsg; i++)
                {
                    fwrite(pkt->msg[i], sizeof(char), strlen(pkt->msg[i]), fp);
                    fwrite("\n", sizeof(char), 1, fp);
                    bzero(buf, MAXLEN);
                }
                fclose(fp);
                printf("File received successful!\n");
                break;
            }
        case MSG_USER_ONLINE:
            {
                char *user_name[100] = {NULL};
                int i = 0;
                for (i = 0; i < pkt->nmsg; i++)     //接收到的信息
                {
                    user_name[i] = strdup(pkt->msg[i]);
                }

                gdk_threads_enter();
                remove_all_user(user_view);
                update_user_online(user_view, user_name);
                //update_user_offline(user_view, user_name);
                gdk_threads_leave();
                printf("MSG_USER_ONLINE:\n");
                break;
            }
        case MSG_USER_ALL:
            {
                printf("MSG_USER_ALL:\n");
                break;
            }
        case MSG_CHECK_USER_ONLINE:
            {
                client_parse_input(cli, "whoison");
                break;
            }
        default:
            {
                for (i = 0; i < pkt->nmsg; i++)     //接收到的信息
                {
                    printf("  %s\n", pkt->msg[i]);
                }
                break;
            }
    }

    packet_free(pkt);
    cli->pktget = NULL;
    printf("> %s", inputbuf);
    fflush(stdout);

    return 0;
}

