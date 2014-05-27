#ifndef _LOGIN_UI_H_
#define _LOGIN_UI_H_

#include "chat.h"
#include <gtk/gtk.h>


void quit_app(GtkWidget *window, gpointer data);
void login_button_clicked (GtkWidget *button, gpointer data);
void list_row_activated(GtkTreeView *treeview,
                      GtkTreePath *path,
                      GtkTreeViewColumn *col,
                      gpointer userdata);
void send_button_clicked(GtkWidget * widget, gpointer data);
void talk_close_button_clicked(GtkWidget * widget, gpointer data);

#endif
