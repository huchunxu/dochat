#include <gtk/gtk.h>

enum {
    COLUMN = 0,
    NUM_COLS
};
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
    {
         gchar *name;

         gtk_tree_model_get(model, &iter, COLUMN, &name, -1);

         g_print ("Double-clicked row contains name %s\n", name);

         g_free(name);
    }
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


int main(int argc, char **argv)
{
    GtkWidget *view;
    GtkWidget *window;
    GtkTreeSelection *selection;
    GtkWidget *vbox;
    GtkWidget *statusbar;
    
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Tree View");
    gtk_widget_set_size_request(window, 200, 500);
    
    vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    view = create_view_and_model();
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
        
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 1);
    
    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, TRUE, 1);
    
    g_signal_connect(view, "row-activated", (GCallback)view_onRowActivated, NULL);
    
    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
}
