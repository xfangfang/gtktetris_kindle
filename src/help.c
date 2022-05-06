#include "tetris.h"

void show_help (void)
{
    GtkWidget *dialog, * button;
    GtkWidget *frame;
    GtkWidget *vbox;
    int i;

    dialog = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (dialog), "L:A_N:application_ID:Help");
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (main_window));
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 3);

    vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL,3);
    gtk_container_add (GTK_CONTAINER (frame),vbox);

    //-----GtkTreeView
    GtkTreeModel     * tree_model;
    GtkTreeView      * treeview;
    GtkTreeSelection * tree_sel;
    GtkTreeIter      iter;
    GtkListStore     * store;

    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    tree_model = GTK_TREE_MODEL (store);

    treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (tree_model));

    gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

    tree_sel = gtk_tree_view_get_selection (treeview);
    gtk_tree_selection_set_mode (tree_sel, GTK_SELECTION_NONE); //_SINGLE _MULTIPLE

    g_object_unref (tree_model);

    static const char * titles[] = { "Keys", "Action" };
    GtkCellRenderer * r[2];
    GtkTreeViewColumn * col[2];
    for (i = 0; i < 2; i++)
    {
        r[i] = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "xalign", 0.0, NULL);
        col[i] = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN,
                               "title",          titles[i],
                               "sort-column-id", i,
                               "alignment",      0.0,
                               "sizing",         GTK_TREE_VIEW_COLUMN_AUTOSIZE,
                               NULL);
        gtk_tree_view_column_pack_start (col[i], r[i], TRUE);
        gtk_tree_view_column_add_attribute (col[i], r[i], "text", i);
        gtk_tree_view_append_column (treeview, col[i]);
    }

    static const char * helpx[] =
    {
        "Right", "move right",
        "Left",  "move left",
        "Up",    "rotate",
        "Down", "drop block",
        "Score: score*level", "",
        "Single", "40",
        "Double", "100",
        "Triple", "300",
        "TETRIS", "1200",
        "Drop bonus: rows*level", "",
         NULL, NULL,
    };

    i = 0;
    while (helpx[i])
    {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            0, helpx[i],
                            1, helpx[i+1],
                            -1);
        i = i+2;
    }

    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (treeview), TRUE, TRUE, 3);
    //-----end of GtkTreeView

    button = gtk_dialog_add_button (GTK_DIALOG (dialog), "gtk-close", GTK_RESPONSE_CLOSE);
    gtk_widget_grab_focus (button);

    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy),
                              (gpointer) dialog);

    gtk_widget_show_all (dialog);
}
