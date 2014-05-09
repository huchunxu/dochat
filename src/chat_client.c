/*==============================================================
 *     FileName: chat_client.c
 *         Desc: 客户端运行主函数
 *       Author: Hu Chunxu
 *        Email: huchunxu@hust.edu.cn
 *      Version: 0.0.1
 *   LastChange: 2014-05-05 16:20:53
 *      History:
 *=============================================================*/
#include "chat.h"

static char *serverip = NULL;
static int serverport = 0;
static char inputbuf[MAXLEN] = {0};  //stdin输入缓存
static int inputs=0;
static int input_handler(ChatClient *cli);
static int input_parse(ChatClient *cli);
static int socket_handler(ChatClient *cli);

void help(char *name)
{
    if (name==NULL){
        return ;
    }
    printf("Usage:\n");
    printf("  %s [config]\n", name);
    printf("Example: %s yichat.conf\n",name);
}

void config_example(void)
{
    printf("please set config file as this!\n");
    printf("name=yiletian\n");
    printf("serverip=192.168.1.45\n");
    printf("serverport=8000\n");
}

int client_config(ChatClient *cli, const char *config)
{
    if (cli==NULL || config==NULL)
    {
        goto FAILED;
    }
    FILE *fp = fopen(config, "r");
    if (fp==NULL)
    {
        printf("[ChatClient]: can't open config file for read\n");
        goto FAILED;
    }

    char buf[MAXLEN] = {0};
    char *key = NULL;
    char *val = NULL;
    char *line = NULL;
    while(fgets(buf, MAXLEN-1, fp))
    {
        line = str_strip(buf);
        if (*line && *line!='#')
        {
            line_parse(line, '=', &key, &val);
            key = str_strip(key);
            val = str_strip(val);
        }else{
            bzero(buf, MAXLEN);
            continue;
        }
        if (key==NULL || val==NULL){
            bzero(buf, MAXLEN);
            continue;
        }

        if (strcmp(key, "name")==0){
            cli->name = strdup(val);
        }else if(strcmp(key, "serverip")==0){
            serverip = strdup(val);
        }else if(strcmp(key, "serverport")==0){
            serverport = atoi(val);
        }
        bzero(buf, MAXLEN);
    }
    if (cli->name==NULL){
        printf("please set \"name\" in client config file\n");
        goto FAILED;
    }
    if (serverip==NULL){
        printf("please set \"serverip\" in client config file\n");
        goto FAILED;
    }
    if(serverport<=0){
        printf("please set \"serverport\" in client config file\n");
        goto FAILED;
    }
    return 0;
FAILED:
    config_example();
    return -1;
}

/**
 * @brief 客户端处理主函数
 *
 * @param argc      输入参数的数量
 * @param argv[]    输入参数值
 *
 * @return 
 */
int main(int argc, char *argv[])
{
    if (argc<2){
        printf("[%s]: please set a config file\n",argv[0]);
        help(argv[0]);
        return -1;
    }

    ChatClient *cli = (ChatClient *)malloc(sizeof(ChatClient));   //客户端分配资源
    if (cli==NULL)
    {
        printf("[%s]: failed to malloc\n", argv[0]);
        return -1;
    }
    bzero(cli, sizeof(ChatClient));
    if(client_config(cli, argv[1])<0){
        return -1;
    }

    cli->sktfd = socket(AF_INET, SOCK_STREAM, 0);             //建立socket
    if(cli->sktfd < 0)
    {
        printf("Client socket error\n");
        return -1;
    }
    cli->sktaddr.sin_family = AF_INET;
    cli->sktaddr.sin_port = htons(serverport);
    inet_pton(AF_INET, serverip, &(cli->sktaddr.sin_addr));   //点分十进制转换成整数
    connect(cli->sktfd, (struct sockaddr *)&(cli->sktaddr), sizeof(cli->sktaddr));

    if(client_login(cli) < 0)
    {
        printf("failed to login\n");
        return -1;
    }
    printf("> ");
    fflush(stdout); 
    cli->epfd = epoll_create(MAXEVENTS);                      //创建监听文件集合
    fd_add_events(cli->epfd, fileno(stdin), EPOLLIN);         //监听标准输入
    fd_add_events(cli->epfd, cli->sktfd, EPOLLIN);            //监听socket

    struct epoll_event evts[MAXEVENTS];
    while(1)
    {
        int fds = epoll_wait(cli->epfd, evts, MAXEVENTS, 100);   //等待终端输入或者socket接收数据
        int i;
        for(i=0;i<fds;i++)
        {
            int fd = evts[i].data.fd;
            if(fd == fileno(stdin))
            {
                input_handler(cli);     //终端输入处理
            }
            else if(fd == cli->sktfd)
            {
                socket_handler(cli);    //socket接受数据处理
            }
        }
    }
    return 0;
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

    if(read(fd, inputbuf+inputs, 1) < 0)  //每次读一个字节
    {
        printf("Stdin input error!\n");
        return -1;
    }

    while(inputbuf[inputs] != '\n')
    {
    		inputs++;
    		if(read(fd, inputbuf+inputs, 1) < 0)  //每次读一个字节
    	    {
    	        printf("Stdin input error!\n");
    	        return -1;
    	    }
    }

    if(inputbuf[inputs] == '\n')          //如果输入换行符，则发送数据
    {
        inputbuf[inputs] = '\0';
        return input_parse(cli);
    }
    //inputs++;
    if(inputs == MAXLEN-1)
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
    char buf[MAXLEN] = {0};
    if(socket_readline(cli->sktfd, buf) < 0)
    {
        printf("Read socket error! maybe the server shutdown!\n");
        exit(0);
    }
    if(client_parse_head(cli, buf) < 0)
    {
        packet_free(cli->pktget);
        cli->pktget = NULL;
        return -1;
    }

    if(cli->pktstat != PKT_RDFULL)
    {
        return 0;
    }

    ChatPacket *pkt = cli->pktget;
    printf("\rFrom: %s\n", pkt->from);
    printf("Time: %s\n", pkt->time);

    int i = 0;
    for(i=0;i<pkt->nmsg;i++)     //发送消息信息
    {
        printf("  %s\n", pkt->msg[i]);
    }
    packet_free(pkt);
    cli->pktget = NULL;
    printf("> %s", inputbuf);
    fflush(stdout);

    return 0;
}

