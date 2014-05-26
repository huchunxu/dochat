#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "chat.h"
#include <gtk/gtk.h>

int login_ui_create();
int main_window_create();
int login_dialog_create();
GtkWidget *talk_window_create();

enum {
    COLUMN = 0,
    NUM_COLS
};

struct login_entry_msg
{
    GtkWidget  *username_entry;
    GtkWidget  *password_entry;
};

extern GtkWidget *main_window;
extern GtkWidget *login_window;
extern struct login_entry_msg login_entry;

#endif
