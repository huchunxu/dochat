#include "chat_parse.h"

static const char *g_head[HEAD_LAST] = {
    [HEAD_START] = "START",
    [HEAD_TO] = "TO:",
    [HEAD_FROM] = "FROM:",
    [HEAD_MSG] = "MSG:",
    [HEAD_TIME] = "TIME:",
    [HEAD_FDBK] = "FDBK:",
    [HEAD_END] = "END",
};

/**
 * @brief 删除字符串首尾的空格
 *
 * @param str 输入字符串
 *
 * @return 
 */
char *str_strip(char *str)
{
    if(str==NULL)
    {
        return NULL;
    }

    int i = 0;
    int len = strlen(str);

    if(str[len-1] == '\n')    //把最后的换行修改成字符串结束符
    {
        str[len-1] = '\0';
        len--;
    }

    if(str[0]!=' ' && str[len-1]!=' ')
    {
        return str;
    }

    while(str[i] == ' ')
    {
        i++;
        if(str[i] == '\0')
            return NULL;
    }

    str += i;
    len -= i;

    while(str[len-1]==' ')
    {
        len--;
    }
    str[len] = '\0';

    return str;
}

/**
 * @brief 将字符串按照分隔符分解
 *
 * @param line   原始字符串
 * @param tok    分隔符
 * @param key    前半段字符串
 * @param val    后半段字符转
 *
 * @return 
 */
int line_parse(char *line, char tok, char **key, char **val)
{
    if(line)
    {
        line = str_strip(line);
    }
    if(line==NULL || key==NULL || val==NULL)
    {
        return -1;
    }

    *key = NULL;
    *val = NULL;

    char *s = strchr(line, tok);    //查找分隔符的位置
    if(s == NULL)
    {
        return -1;
    }

    *s = '\0';
    s++;
    *key = str_strip(line);
    *val = str_strip(s);

    return 0;
}

/**
 * @brief 判断收到数据的类型
 *
 * @param head 数据
 *
 * @return 
 */
HeadType packet_head_parse(const char *head)
{
    if (head==NULL || *head=='\0'){
        return HEAD_LAST;
    }

    HeadType type;
    for (type = 0; type<HEAD_LAST; type++){
        int len = strlen(g_head[type]);
        if(strncmp(head, g_head[type], len)==0){
            break;
        }
    }

    return type;
}

/**
 * @brief 接收数据
 *
 * @param type 数据类型
 * @param head 数据
 *
 * @return 
 */
char *packet_head_getdata(HeadType type, char *head)
{
    if (type>=HEAD_LAST || g_head[type]==NULL){
        return NULL;
    }

    int len = strlen(g_head[type]);

    char *str = head + len;
    while(*str==' ')
        str++;
    return str;       
}

/**
 * @brief 把消息添加到消息结构中
 *
 * @param pkt 消息结构
 * @param msg 发送的消息
 *
 * @return 
 */
int packet_add_msg(ChatPacket *pkt, const char *msg)
{
    if (pkt->nmsg>=MAXMSG || msg==NULL){
        return -1;
    }

    pkt->msg[pkt->nmsg] = strdup(msg);
    pkt->nmsg++;

    return 0;
}

/**
 * @brief 发送消息包中的一个数据项
 *
 * @param sktfd  socket
 * @param type   数据项标志
 * @param data   数据
 *
 * @return 
 */
int socket_send_head(int sktfd, HeadType type, const char *data)
{
    char buf[MAXLEN] = {0};

    if(type==HEAD_START || type==HEAD_END)
    {
        sprintf(buf, "%s", g_head[type]);
    }
    else if(((data==NULL) || (*data=='\0')) || (type>=HEAD_LAST))
    {
        return 0;
    }
    else
    {
        sprintf(buf, "%s%s\n", g_head[type], data);
    }

    if(socket_writeline(sktfd, buf)<0)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 创建消息包，并且复制发起者和接收者
 *
 * @param from  发起者
 * @param to    接收者
 *
 * @return 新建立的消息包
 */
ChatPacket *packet_new(const char *from, const char *to)
{
    ChatPacket *pkt = (ChatPacket *)malloc(sizeof(ChatPacket));
    bzero(pkt, sizeof(ChatPacket));

    if(from)
    {
        pkt->from = strdup(from);
    }
    if(to)
    {
        pkt->to = strdup(to);
    }
    return pkt;
}

