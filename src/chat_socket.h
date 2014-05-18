#ifndef _CHAT_SOCKET_H_
#define _CHAT_SOCKET_H_

#include "chat.h"
#include "chat_parse.h"


int chat_add_events(ChatClient *cli, int events);
int fd_add_events(int epfd, int fd, int events);
int client_login(ChatClient *cli);
int client_register(ChatClient *cli);
int client_parse_input(ChatClient *cli, char *input);
int client_flush(ChatClient *cli);
int socket_writeline(int sktfd, char *line);
int socket_readline(int sktfd, char *buf);
int packet_free(ChatPacket *pkt);
char *gettime(void);
int client_parse_head(ChatClient *cli, char *head);
int chat_remove_events(ChatClient *cli);
ServerCmd server_parse_cmd(char *msg);
int send_to_client(ChatClient *cli, const char *msg);


#endif
