#ifndef _CHAT_CLIENT_H_
#define _CHAT_CLIENT_H_

#include "chat.h"
#include "chat_parse.h"

static int input_handler(ChatClient *cli);
static int input_parse(ChatClient *cli);
static int socket_handler(ChatClient *cli);
static int para_analyze(ChatClient *cli, int argc, char *argv[]);
static void help();

#endif
