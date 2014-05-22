/*==============================================================
 *     FileName: callback.c
 *         Desc: 界面的回调函数
 *       Author: Hu Chunxu
 *        Email: huchunxu@hust.edu.cn
 *      Version: 0.0.1
 *   LastChange: 2014-05-22 16:20:58
 *      History:
 *=============================================================*/
#include "callback.h"
#include "interface.h"
const char * password = "secret";
extern GtkWidget *main_window;
extern GtkWidget *login_window;
extern GtkWidget *username_entry, *password_entry;
/**
 * @brief 退出
 *
 * @param window
 * @param data
 */
void quit_app(GtkWidget *window, gpointer data)
{
    gtk_main_quit();
}


/**
 * @brief 登陆按键的回调函数
 *
 * @param button
 * @param data
 */
void login_button_clicked (GtkWidget *button, gpointer data)
{
    const char *password_text = gtk_entry_get_text(GTK_ENTRY(password_entry));
    const char *username_text = gtk_entry_get_text(GTK_ENTRY(username_entry));

    if (strcmp(password_text, password) == 0)
    {
        printf("Access granted!\n");
        
        main_window_create();
        gtk_widget_show_all(main_window);
        gtk_widget_hide_all(login_window);
    }
    else
    {
        printf("Access denied!\n");
        login_dialog_create();
    }
}

void view_onRowActivated (GtkTreeView *treeview,
                      GtkTreePath *path,
                      GtkTreeViewColumn *col,
                      gpointer userdata)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_print ("A row has been double-clicked!\n");

    model = gtk_tree_view_get_model(treeview);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {Access granted!
A row has been double-clicked!
Double-clicked row contains name
         gchar *name;

         gtk_tree_model_get(model, &iter, COLUMN, &name, -1);

         gtk_statusbar_push(GTK_STATUSBAR(userdata), gtk_statusbar_get_context_id(GTK_STATUSBAR(userdata),  
                                   name), name); 
         g_print ("Double-clicked row contains name %s\n", name);
         g_free(name);
    }
}

