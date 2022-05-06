/* 
 * highscore dialog and functions
 * it's briefly used in interface.c
 */

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "tetris.h"

struct item
{
   char name[10];
   long score;
   int level;
   int lines;
};

struct item highscore[NUM_HIGHSCORE];

void read_highscore()
{
   FILE *fp;
   char * hfile = get_config_dir_file ("highscore.dat");
   if ((fp = fopen (hfile,"r")))
   {
      fread (&highscore, 1, sizeof(highscore),fp);
      fclose (fp);
   }
   g_free (hfile);
}

void write_highscore()
{
   FILE *fp;
   char * hfile = get_config_dir_file ("highscore.dat");
   if ((fp = fopen (hfile,"w"))) {
      fwrite (&highscore, 1, sizeof(highscore),fp);
      fclose (fp);
   }
   g_free (hfile);
}


void show_highscore_dlg ()
{
   GtkWidget * dialog;
   GtkWidget *vbox, * button;
   int i, temp;

   dialog = gtk_dialog_new ();
   gtk_window_set_title (GTK_WINDOW (dialog), "L:A_N:application_ID:Highscores");
   gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (main_window));
   gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
   gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
   gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
   gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
   gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
   //gtk_container_set_border_width (GTK_CONTAINER (dialog), 3);

   vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

   //-----GtkTreeView
   GtkTreeModel     * tree_model;
   GtkTreeView      * treeview;
   GtkTreeSelection * tree_sel;
   GtkTreeIter      iter;
   GtkListStore     * store;

   store = gtk_list_store_new (5,
                               G_TYPE_INT,    /*   #   */
                               G_TYPE_STRING, /* name  */
                               G_TYPE_INT,    /* lines */
                               G_TYPE_INT,    /* level */
                               G_TYPE_INT);   /* score */
   tree_model = GTK_TREE_MODEL (store);

   treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (tree_model));

   gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

   tree_sel = gtk_tree_view_get_selection (treeview);
   gtk_tree_selection_set_mode (tree_sel, GTK_SELECTION_NONE); //_SINGLE _MULTIPLE

   g_object_unref (tree_model);

   /* COLUMNS */
   static const char * titles[] = { "#", "Name", "Lines", "Level", "Score" };
   GtkCellRenderer * r[5];
   GtkTreeViewColumn * col[5];
   for (i = 0; i < 5; i++)
   {
      r[i] = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "xalign", 0.5, NULL);
      col[i] = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN,
                             "title",          titles[i],
                             "sort-column-id", i,
                             "alignment",      0.5,
                             "sizing",         GTK_TREE_VIEW_COLUMN_AUTOSIZE,
                             NULL);
      gtk_tree_view_column_pack_start (col[i], r[i], TRUE);
      gtk_tree_view_column_add_attribute (col[i], r[i], "text", i);
      gtk_tree_view_append_column (treeview, col[i]);
   }

   for(temp = 0; temp < NUM_HIGHSCORE; temp++)
   {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          0, temp+1,
                          1, highscore[temp].name,
                          2, highscore[temp].lines,
                          3, highscore[temp].level,
                          4, highscore[temp].score,
                          -1);
   }

   gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (treeview), TRUE, TRUE, 3);
   //-----End of GtkTreeView

   button = gtk_dialog_add_button (GTK_DIALOG (dialog), "gtk-close", GTK_RESPONSE_CLOSE);
   gtk_widget_grab_focus (button);

   g_signal_connect_swapped (dialog, "response",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) dialog);

   gtk_widget_show_all(dialog);
}


int addto_highscore (char *name, long score, int level, int lines)
{
   int place = 0;
   int temp, namelen;
   int added = FALSE;

   for (temp=NUM_HIGHSCORE-1; temp > -1; temp--)
   {
      if (score > highscore[temp].score || (highscore[temp].score == 0 && strlen(highscore[temp].name) == 0))
      {
         place = temp;
         added = TRUE;
      }
   }
   if (added)
   {
      for(temp=NUM_HIGHSCORE-1; temp > place; temp--) {
         memcpy (&highscore[temp], &highscore[temp-1], sizeof(highscore[0]));
      }
      namelen = strlen (name);
      if(namelen > 9) {
         namelen = 9;
      }
      memset (&highscore[place].name, 0, sizeof(highscore[0].name));
      memcpy (&highscore[place].name, name,namelen);
      highscore[place].score = score;
      highscore[place].level = level;
      highscore[place].lines = lines;
   }
   return (place+1);
}

long get_hiscore(void)
{
   return (highscore[0].score);
}
