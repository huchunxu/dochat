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
GtkWidget *user_view;

struct login_entry_msg login_entry;
struct talk_msg allmsg[10];

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

    login_entry.username_entry = gtk_entry_new();
    login_entry.password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY (login_entry.password_entry), FALSE);	

    ok_button = gtk_button_new_with_label("登陆");

    g_signal_connect (GTK_OBJECT (ok_button), "clicked", GTK_SIGNAL_FUNC(login_button_clicked), NULL);

    hbox1 = gtk_hbox_new(TRUE, 5);
    hbox2 = gtk_hbox_new(TRUE, 5);

    vbox = gtk_vbox_new(FALSE, 10);

    //用户名box
    gtk_box_pack_start(GTK_BOX(hbox1), username_label, TRUE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox1), login_entry.username_entry, TRUE, FALSE, 5);

    //密码box
    gtk_box_pack_start(GTK_BOX(hbox2), password_label, TRUE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox2), login_entry.password_entry, TRUE, FALSE, 5);

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
    gtk_tree_store_set(treestore, &toplevel, COLUMN, "在线好友", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "Python", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "Perl", -1);
    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child, COLUMN, "PHP", -1);

    gtk_tree_store_append(treestore, &toplevel, NULL);
    gtk_tree_store_set(treestore, &toplevel, COLUMN, "离线好友", -1);
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

void update_user_online(GtkWidget *list, char *user_online[])
{
    GtkListStore *store;
    GtkTreeIter toplevel, child;
    int i = 0;

    store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (list)));

    gtk_tree_store_append(store, &toplevel, NULL);
    gtk_tree_store_set(store, &toplevel, COLUMN, "在线好友", -1);

    while(user_online[i] != NULL)
    {
        gtk_tree_store_append(store, &child, &toplevel);
        gtk_tree_store_set(store, &child, COLUMN, user_online[i], -1);
        i++;
    }
}

void update_user_offline(GtkWidget *list, char *user_offline[])
{
    GtkListStore *store;
    GtkTreeIter toplevel, child;
    int i = 0;

    store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (list)));

    gtk_tree_store_append(store, &toplevel, NULL);
    gtk_tree_store_set(store, &toplevel, COLUMN, "离线好友", -1);
      
    while(user_offline[i] != NULL)
    {
        gtk_tree_store_append(store, &child, &toplevel);
        gtk_tree_store_set(store, &child, COLUMN, user_offline[i], -1);
        i++;
    }
}


void remove_all_user(GtkWidget *list)
{
  GtkListStore *store;
  GtkTreeModel *model;
  GtkTreeIter  iter;

  store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (list)));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));

  if (gtk_tree_model_get_iter_first(model, &iter) == FALSE) 
      return;
  gtk_tree_store_clear(store);
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
    user_view = create_view_and_model();
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(user_view));
   
    gtk_box_pack_start(GTK_BOX(main_window_vbox), user_view, TRUE, TRUE, 1);

    //创建状态栏
    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(main_window_vbox), statusbar, FALSE, TRUE, 1);

    g_signal_connect(user_view, "row-activated", (GCallback) list_row_activated, statusbar);
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

/**
 * @brief 聊天界面的设计
 *
 * @return 
 */
GtkWidget *talk_window_create(char *name)
{
    GtkWidget *window;
    GtkWidget *label1;
    GtkWidget *list1;
    GtkWidget *label2;
    GtkWidget *list2;
    GtkWidget *label3;
    GtkWidget *button_send;
    GtkWidget *button_close;
    GtkWidget *table;

    GtkWidget *send_file;
    GtkWidget *image4;
    GtkWidget *xiexin;
    GtkWidget *image5;
    GtkWidget *xiaoxi;
    GtkWidget *image6;

    GdkColor color;
    color.red = 0x9e00;
    color.green = 0xcf00;
    color.blue = 0xff00;   
    //	gtk_widget_modify_bg(window_login,GTK_STATE_NORMAL, &color);

    GtkWidget *scrolledwindow1;
    char acp[100];
    sprintf(acp, "正在与 %s 进行对话聊天", name);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
    gtk_widget_set_size_request(window, 401, 350);
    //	gtk_window_set_default_size(GTK_WINDOW(window), 200, 180);
    gtk_window_set_title(GTK_WINDOW(window), acp);
    gtk_container_set_border_width(GTK_CONTAINER(window),3);
    //gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(delete_event), (gpointer) h);

    gtk_widget_modify_bg(window,GTK_STATE_NORMAL, &color);

    table = gtk_table_new(9, 10, TRUE);
    gtk_container_add(GTK_CONTAINER(window), table);
    gtk_widget_show(table);

    label1 = gtk_label_new("1");
    gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 10, 0, 1);
    gtk_widget_show(label1);

    list1 = gtk_text_view_new();
    gtk_table_attach_defaults(GTK_TABLE(table), list1, 0, 10, 1, 5);
    gtk_widget_show(list1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(list1), FALSE);

    label2 = gtk_label_new("2");
    gtk_table_attach_defaults(GTK_TABLE(table), label2,0,4, 5, 6);
    gtk_widget_show(label2);
    //中间按钮工具栏+++++++++++
    send_file = gtk_button_new ();
    gtk_widget_show (send_file);

    image4 =gtk_image_new_from_file("./ico/QQ截图未命名.gif");
    gtk_widget_show (image4);

    gtk_container_add (GTK_CONTAINER (send_file), image4);
    gtk_widget_set_size_request (image4, 20, -1);
    gtk_table_attach_defaults(GTK_TABLE(table),send_file,4,6, 5, 6);

    xiexin = gtk_button_new ();
    gtk_widget_show (xiexin);
    gtk_widget_set_size_request (xiexin, 25, 32);

    //	image5 = create_pixmap (window, "写信.gif");
    image5 =gtk_image_new_from_file("./ico/写信.gif");
    gtk_widget_show (image5);
    gtk_container_add (GTK_CONTAINER (xiexin), image5);
    gtk_widget_set_size_request (image5,-1, 20);
    gtk_table_attach_defaults(GTK_TABLE(table),xiexin, 6,8, 5, 6);

    xiaoxi = gtk_button_new ();
    gtk_widget_show (xiaoxi);
    gtk_widget_set_size_request (xiaoxi, -1, 32);

    //	image6 = create_pixmap (window, "聊天记录.jpg");
    image6 =gtk_image_new_from_file("./ico/聊天记录.jpg");
    gtk_widget_show (image6);
    gtk_container_add (GTK_CONTAINER (xiaoxi), image6);
    gtk_widget_set_size_request (image6, -1, 20);
    gtk_table_attach_defaults(GTK_TABLE(table),xiaoxi, 8,10, 5, 6);
    //+++++++++++

    scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow1);
    ///	gtk_container_add (GTK_CONTAINER (window2), scrolledwindow1);
    //	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

    list2 = gtk_text_view_new();
    gtk_table_attach_defaults(GTK_TABLE(table), list2, 0, 10, 6, 8);
    gtk_widget_show(list2);

    gtk_container_add (GTK_CONTAINER (scrolledwindow1),list2);

    gtk_table_attach_defaults(GTK_TABLE(table),scrolledwindow1, 0, 10, 6, 8);
    //++++++++++++++
    label3 = gtk_label_new("");
    gtk_table_attach_defaults(GTK_TABLE(table), label3, 0, 6, 8, 9);
    gtk_widget_show(label3);

    button_send = gtk_button_new_with_mnemonic(" 发送(_s) ");
    gtk_table_attach_defaults(GTK_TABLE(table), button_send, 6, 8, 8, 9);
    gtk_widget_show(button_send);

    button_close = gtk_button_new_with_mnemonic(" 关闭(_c) ");
    gtk_table_attach_defaults(GTK_TABLE(table), button_close, 8, 10, 8, 9);
    gtk_widget_show(button_close);

    //allmess[h].listup = list1;
    //allmess[h].listdown = list2;
    //allmess[h].buffer_up = gtk_text_view_get_buffer(GTK_TEXT_VIEW(list1));
    //allmess[h].buffer_down = gtk_text_view_get_buffer(GTK_TEXT_VIEW(list2));
    //gtk_signal_connect (GTK_OBJECT (button_close),"clicked",GTK_SIGNAL_FUNC(delete_event),(gpointer)h);

    //struct close klose;
    //klose.wcl=window;
    //klose.cl=h;

    allmsg[0].buffer_up = gtk_text_view_get_buffer(GTK_TEXT_VIEW(list1));
    allmsg[0].buffer_down = gtk_text_view_get_buffer(GTK_TEXT_VIEW(list2));
    allmsg[0].to_name = strdup(name);
    int num = 0;
   	gtk_signal_connect(GTK_OBJECT(button_send), "clicked", GTK_SIGNAL_FUNC(send_button_clicked),(gpointer)num);
    //	gtk_signal_connect(GTK_OBJECT(xiaoxi), "clicked", GTK_SIGNAL_FUNC(button_recourd),(gpointer) h);
    //	gtk_signal_connect(GTK_OBJECT(xiexin), "clicked", GTK_SIGNAL_FUNC(button_email),(gpointer) h);
   	gtk_signal_connect(GTK_OBJECT(button_close), "clicked", GTK_SIGNAL_FUNC(talk_close_button_clicked), (gpointer)window);
    //	gtk_signal_connect(GTK_OBJECT(send_file), "clicked", GTK_SIGNAL_FUNC(on_button_clicked),(gpointer) h);

    gtk_widget_show_all(window);

    return window;

}


int main(int argc, char *argv[])
{
    gdk_threads_init ();

    gtk_init (&argc, &argv);

    login_ui_create();

    gtk_main ();

    return 0;
}

