#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "chat.h"
#include <gtk/gtk.h>

int login_ui_create();
int main_window_create();
int login_dialog_create();

enum {
    COLUMN = 0,
    NUM_COLS
};


#endif
