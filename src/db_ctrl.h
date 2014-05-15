#ifndef _DB_CTRL_H_
#define _DB_CTRL_H_

#include <stdio.h>
#include <string.h>

#include "mysql.h"


#define ERR_DB   -1
#define ERR_USER -2
#define ERR_PWD  -3
#define ERR_MATCH -4

int database_start(char *name, char *pwd);
void database_end();
int search_user(char *user_name);
int create_user(char *user_name, char *pwd);
int match_user(char *user_name, char *pwd);
int check_user(char *user_name, char *pwd);
int register_user(char *user_name, char *pwd);

#endif
