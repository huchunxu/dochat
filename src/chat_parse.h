#ifndef _CHAT_PARSE_H_
#define _CHAT_PARSE_H_

#include "chat.h"
#include "chat_socket.h"

char *str_strip(char *str);
int line_parse(char *line, char tok, char **key, char **val);
HeadType packet_head_parse(const char *head);
char *packet_head_getdata(HeadType type, char *head);
int packet_add_msg(ChatPacket *pkt, const char *msg);
int socket_send_head(int sktfd, HeadType type, const char *data);
ChatPacket *packet_new(const char *from, const char *to);
MsgType packet_msg_parse(const char *msg_type);
char *get_msg_type(MsgType type);

#endif
