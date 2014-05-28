#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "chat.h"
#include <gtk/gtk.h>

void login_ui_create();
void main_window_create();
void login_dialog_create();
GtkWidget *talk_window_create(char *from_name, char *name);
void remove_all_user(GtkWidget *list);
void update_user_offline(GtkWidget *list, char *user_offline[]);
void update_user_online(GtkWidget *list, char *user_ooline[]);

enum {
    COLUMN = 0,
    NUM_COLS
};

struct login_entry_msg
{
    GtkWidget  *username_entry;
    GtkWidget  *password_entry;
};

struct talk_msg
{
    GtkTextBuffer *buffer_up;
    GtkTextBuffer *buffer_down;
    char* to_name;
    int open_state;
};

extern GtkWidget *main_window;
extern GtkWidget *login_window;
extern struct login_entry_msg login_entry;
extern GtkWidget *user_view;
extern struct talk_msg allmsg[10];
extern int talk_window_num;

#endif
