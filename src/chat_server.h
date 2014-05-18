#ifndef _CHAT_SERVER_H_
#define _CHAT_SERVER_H_

#include "chat.h"
#include "db_ctrl.h"
#include "chat_parse.h"
#include "chat_socket.h"


#define DEF_PORT 8000   //默认端口

static int server_init(ChatServer *serv);
static int server_run(ChatServer *serv);
static int server_add_client(ChatServer *serv, ChatClient *cli);
static int server_remove_client(ChatServer *serv, ChatClient *cli);
static void *server_event_thread(void *data);
static int server_get_msg(ChatServer *serv, ChatClient *cli);
static int server_send_msg(ChatServer *serv, ChatClient *cli);
static int server_packet_handler(ChatServer *serv, ChatClient *cli);
static ChatClient *server_find_client(ChatServer *serv, const char *name);


#endif
