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
#include "chat_client.h"

static char *serverip = "127.0.0.1";
static int serverport = 8000;

const char * password = "secret";

ChatClient *cli;

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


void test()
{
    int counter = 0;
    while(1)
    {
        printf("%d\n",counter++);
        usleep(1000000);
    }
}
/**
 * @brief 登陆按键的回调函数
 *
 * @param button
 * @param data
 */
void login_button_clicked (GtkWidget *button, gpointer data)
{
    const char *password_text = gtk_entry_get_text(GTK_ENTRY(login_entry.password_entry));
    const char *username_text = gtk_entry_get_text(GTK_ENTRY(login_entry.username_entry));
 
    cli = (ChatClient *) malloc(sizeof(ChatClient));   //客户端分配资源
    if (cli == NULL)
    {
        printf("[%s]: failed to malloc\n", username_text);
        return -1;
    }
    bzero(cli, sizeof(ChatClient));
    cli->name = strdup(username_text);
    cli->password = strdup(password_text);
 
    cli->sktfd = socket(AF_INET, SOCK_STREAM, 0);             //建立socket
    if (cli->sktfd < 0)
    {
        printf("Client socket error\n");
        return -1;
    }
    cli->sktaddr.sin_family = AF_INET;
    cli->sktaddr.sin_port = htons(serverport);
    inet_pton(AF_INET, serverip, &(cli->sktaddr.sin_addr));   //点分十进制转换成整数
    connect(cli->sktfd, (struct sockaddr *) &(cli->sktaddr), sizeof(cli->sktaddr));

    if (client_login(cli) < 0)
    {
        printf("failed to login\n");
        return -1;
    }
 
    printf("> ");
    fflush(stdout);
    cli->epfd = epoll_create(MAXEVENTS);                      //创建监听文件集合
    fd_add_events(cli->epfd, fileno(stdin), EPOLLIN);         //监听标准输入
    fd_add_events(cli->epfd, cli->sktfd, EPOLLIN);            //监听socket
 
    g_thread_new("client_main", (GThreadFunc)client_main,  cli);
}

void list_row_activated(GtkTreeView *treeview,
        GtkTreePath *path,
        GtkTreeViewColumn *col,
        gpointer userdata)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model(treeview);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        gchar *name;

        gtk_tree_model_get(model, &iter, COLUMN, &name, -1);
        
        if(strcmp(name, "在线好友") != 0)
        {
            talk_window_create(name);
        }
        gtk_statusbar_push(GTK_STATUSBAR(userdata), 
                gtk_statusbar_get_context_id(GTK_STATUSBAR(userdata), name), name); 
        g_print ("Double-clicked row contains name %s\n", name);
        g_free(name);
    }
}

void send_button_clicked(GtkWidget * widget, gpointer data)
{
	char *text;
    int num = (int)data;
	GtkTextIter start, end;

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(allmsg[num].buffer_down), &start, &end);	/*获得缓冲区开始和结束位置的Iter */
	const GtkTextIter s = start, e = end;
	text = (char *)gtk_text_buffer_get_text(GTK_TEXT_BUFFER(allmsg[num].buffer_down), &s, &e, FALSE);	/*获得文本框缓冲区文本 */
    char send_text[MAXLEN];
    sprintf(send_text, "to %s:", allmsg[0].to_name);
    strcat(send_text, text);    
    printf("%s\n",send_text);
    client_parse_input(cli, send_text);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(allmsg[0].buffer_down), "", -1);  //清空缓冲区
}

void talk_close_button_clicked(GtkWidget * widget, gpointer data)
{
    gtk_widget_destroy((GtkWidget *)data);
}
