/*==============================================================
 *     FileName: interface.c
 *         Desc: GUI界面设计
 *       Author: Hu Chunxu
 *        Email: huchunxu@hust.edu.cn
 *      Version: 0.0.1
 *   LastChange: 2014-05-22 15:58:54
 *      History:
 *=============================================================*/
#include "interface.h"
#include "callback.h"

GtkWidget *main_window;
GtkWidget *login_window;
GtkWidget *username_entry, *password_entry;


/**
 * @brief 登陆界面的设计
 *
 * @return 
 */
int login_ui_create()
{    
    int login_window_width  = 200;   // 登陆界面的尺寸
    int login_window_length = 150;

    GtkWidget *ok_button;
    GtkWidget *hbox1, *hbox2;
    GtkWidget *vbox;
    GtkWidget *username_label, *password_label;

    login_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(login_window), "登陆");
    gtk_window_set_position(GTK_WINDOW(login_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(login_window), login_window_width, login_window_length);

    g_signal_connect (GTK_OBJECT(login_window), "destroy", GTK_SIGNAL_FUNC (quit_app), NULL);

    username_label = gtk_label_new("用户名:");
    password_label = gtk_label_new("密码:");

    username_entry = gtk_entry_new();
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY (password_entry), FALSE);	

    ok_button = gtk_button_new_with_label("登陆");

    g_signal_connect (GTK_OBJECT (ok_button), "clicked", GTK_SIGNAL_FUNC(login_button_clicked), NULL);

    hbox1 = gtk_hbox_new(TRUE, 5);
    hbox2 = gtk_hbox_new(TRUE, 5);

    vbox = gtk_vbox_new(FALSE, 10);

    //用户名box
    gtk_box_pack_start(GTK_BOX(hbox1), username_label, TRUE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox1), username_entry, TRUE, FALSE, 5);

    //密码box
    gtk_box_pack_start(GTK_BOX(hbox2), password_label, TRUE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox2), password_entry, TRUE, FALSE, 5);

    //将所有box整合
    gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), ok_button, FALSE, FALSE, 5);

    //添加到window中，并且显示
    gtk_container_add(GTK_CONTAINER(login_window), vbox);
    gtk_widget_show_all(login_window);

    return 0;
}

static GtkTreeModel *create_and_fill_model(void)
{
    GtkTreeStore *treestore;    
    GtkTreeIter toplevel, child;

    treestore = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING);

    gtk_tree_store_append(treestore, &toplevel, NULL);
    gtk_tree_store_set(treestore, &toplevel, COLUMN, "我的好友", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "Python", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "Perl", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "PHP", -1);

    gtk_tree_store_append(treestore, &toplevel, NULL);
    gtk_tree_store_set(treestore, &toplevel, COLUMN, "陌生人", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "C", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "C++", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "Java", -1);

    gtk_tree_store_append(treestore, &toplevel, NULL);
    gtk_tree_store_set(treestore, &toplevel, COLUMN, "黑名单", -1);

    return GTK_TREE_MODEL(treestore);
}

static GtkWidget *create_view_and_model(void)
{
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;
    GtkWidget *view;
    GtkTreeModel *model;

    view = gtk_tree_view_new();

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "好友列表");
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", COLUMN);

    model = create_and_fill_model();
    gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);
    g_object_unref(model);

    return view;
}

/**
 * @brief 主界面的设计
 *
 * @return 
 */
int main_window_create()
{
    int main_window_width  = 200;   //主界面的尺寸
    int main_window_length = 600;

    GtkWidget *view;
    GtkTreeSelection *selection;
    GtkWidget *main_window_vbox;
    GtkWidget *statusbar;

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "DoChat");
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(main_window), main_window_width, main_window_length);

    g_signal_connect(GTK_OBJECT(main_window), "destroy", GTK_SIGNAL_FUNC(quit_app), NULL);

    //创建垂直布局构件  
    main_window_vbox = gtk_vbox_new (FALSE, 2);  
    gtk_container_add (GTK_CONTAINER (main_window), main_window_vbox);  

    //创建TreeView
    view = create_view_and_model();
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
        
    gtk_box_pack_start(GTK_BOX(main_window_vbox), view, TRUE, TRUE, 1);
   
    //创建状态栏
    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(main_window_vbox), statusbar, FALSE, TRUE, 1);
    
    g_signal_connect(view, "row-activated", (GCallback) view_onRowActivated, statusbar);
}

/**
 * @brief 登陆错误界面设计
 *
 * @return 
 */
int login_dialog_create()
{
    GtkWidget *dialog = gtk_message_dialog_new(
             GTK_WINDOW(login_window),                   //没有父窗口
             GTK_DIALOG_DESTROY_WITH_PARENT,             //跟随父窗口关闭
             GTK_MESSAGE_ERROR,                          //显示警告图标
             GTK_BUTTONS_OK,                             //显示OK按钮
            "用户名或密码错误!");

    gtk_window_set_title(GTK_WINDOW(dialog), "错误");
    gtk_dialog_run(GTK_DIALOG(dialog));                 //运行对话框
    gtk_widget_destroy(dialog);                         //删除对话框
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    login_ui_create();

    gtk_main ();
}
