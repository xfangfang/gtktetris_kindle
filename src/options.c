/*
 * preferences stored in a file
 */

#include "tetris.h"
#include <ctype.h> /* isdigit */

#define OPTIONS_FILE "config.ini"

static GtkWidget * settings_dialog;
static GtkWidget * spin_level;
static GtkWidget * spin_noise_level;
static GtkWidget * spin_noise_height;
static GtkWidget * show_block_chk;
static GtkWidget * show_grid_chk;
static GtkWidget * block_size_combo;
static GtkWidget * radio_style1;
static GtkWidget * radio_style2;
static const int block_sizes[] = {
   16, 24, 32, 0
};
static const char * block_sizes_str[] = {
   "16", "24", "32", NULL
};


void options_defaults (void)
{
   options.start_level = 0;
   options.noise_level = 0;
   options.noise_height = 0;
   options.show_next_block = 1;
   options.show_grid = 1;
   options.block_size = 32;
   options.block_style = 1;
}


static void options_save (void)
{
   char * options_f = get_config_dir_file (OPTIONS_FILE);
   GKeyFile * kfile;

   kfile = g_key_file_new ();
   g_key_file_set_integer (kfile, "settings", "start_level", options.start_level);
   g_key_file_set_integer (kfile, "settings", "noise_level", options.noise_level);
   g_key_file_set_integer (kfile, "settings", "noise_height", options.noise_height);
   g_key_file_set_integer (kfile, "settings", "show_next_block", options.show_next_block);
   g_key_file_set_integer (kfile, "settings", "show_grid", options.show_grid);
   g_key_file_set_integer (kfile, "settings", "block_size", options.block_size);
   g_key_file_set_integer (kfile, "settings", "block_style", options.block_style);
   g_key_file_save_to_file (kfile, options_f, NULL);
   g_key_file_free (kfile);

   g_free (options_f);
}


void options_read (void)
{
   char * options_f = get_config_dir_file (OPTIONS_FILE);
   GKeyFile * kfile;

   kfile = g_key_file_new ();
   if (g_key_file_load_from_file (kfile, options_f,
                                  G_KEY_FILE_NONE, NULL))
   {
      options.start_level = g_key_file_get_integer (kfile, "settings", "start_level", NULL);
      options.noise_level = g_key_file_get_integer (kfile, "settings", "noise_level", NULL);
      options.noise_height = g_key_file_get_integer (kfile, "settings", "noise_height", NULL);
      options.show_next_block = g_key_file_get_integer (kfile, "settings", "show_next_block", NULL);
      options.show_grid = g_key_file_get_integer (kfile, "settings", "show_grid", NULL);
      options.block_size = g_key_file_get_integer (kfile, "settings", "block_size", NULL);
      if (g_key_file_has_key (kfile, "settings", "block_style", NULL)) {
         options.block_style = g_key_file_get_integer (kfile, "settings", "block_style", NULL);
      }
      if (options.block_size < 16)
          options.block_size = 24;
      if (options.block_style != 1 && options.block_style != 2)
          options.block_style = 1;
   }
   g_key_file_free (kfile);

   g_free (options_f);
}


static void settings_dialog_response_cb (GtkDialog * dialog,
                                         int response, gpointer user_data)
{
   if (response == GTK_RESPONSE_OK)
   {
      // validate block size...
      GtkWidget * entry = gtk_bin_get_child (GTK_BIN (block_size_combo));
      const char * block_size = gtk_entry_get_text (GTK_ENTRY (entry));
      const char * p = block_size;
      int error = 0;
      if (!*p) error = 1;
      while (*p) {
         if (!isdigit (*p)) {
            error = 1;
            break;
         }
         p++;
      }
      if (error) {
         g_signal_stop_emission_by_name (dialog, "response");
         GtkWidget * m;
         m = gtk_message_dialog_new (GTK_WINDOW (dialog),
                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_OK,
                                     "Invalid block size");
         g_signal_connect_swapped (m, "response", G_CALLBACK (gtk_widget_destroy), m);
         gtk_window_set_title (GTK_WINDOW (m), "L:A_N:application_ID:game_over");
         gtk_widget_show (m);
         return;
      }

      options.block_size = strtoll (block_size, NULL, 10);
      options.start_level = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin_level));
      options.noise_level = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin_noise_level));
      options.noise_height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin_noise_height));
      options.show_next_block = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_block_chk));
      options.show_grid = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_grid_chk));
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_style1))) {
         options.block_style = 1;
      } else {
         options.block_style = 2;
      }
      current_level = options.start_level;
      options_save ();

      update_block_size (0);
   }

   gtk_widget_destroy (GTK_WIDGET (dialog));
}


void options_show_dialog (void)
{
  GtkWidget * frame, * hbox, * label, * radio;
  GSList * radio_group;
  GtkWidget * vbox, * checkbox, * button;
  GtkWidget * vbox_table;
  GtkWidget * hbox_row[3], * labels[3], * spins[3];
  GtkAdjustment * adjs[3];
  int i, combo_active = -1;

  settings_dialog = gtk_dialog_new ();

  gtk_window_set_title(GTK_WINDOW(settings_dialog),"L:A_N:application_ID:Settings");
  gtk_window_set_transient_for (GTK_WINDOW (settings_dialog), GTK_WINDOW (main_window));
  gtk_window_set_position (GTK_WINDOW (settings_dialog), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_modal (GTK_WINDOW (settings_dialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (settings_dialog), FALSE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (settings_dialog), TRUE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (settings_dialog), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (settings_dialog), 4);

  vbox = gtk_dialog_get_content_area (GTK_DIALOG (settings_dialog));
  gtk_box_set_spacing (GTK_BOX (vbox), 2);

  // -------------
  // vbox -> frame
  frame = gtk_frame_new ("Blocks");
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  // vbox -> frame -> vbox
  vbox_table = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_table), 5); // padding
  gtk_container_add (GTK_CONTAINER (frame), vbox_table);

  checkbox = gtk_check_button_new_with_mnemonic ("Show _next block");
  gtk_box_pack_start (GTK_BOX (vbox_table), checkbox, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), options.show_next_block);
  show_block_chk = checkbox;

  checkbox = gtk_check_button_new_with_mnemonic ("Show _grid");
  gtk_box_pack_start (GTK_BOX (vbox_table), checkbox, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), options.show_grid);
  show_grid_chk = checkbox;

  // vbox -> frame -> vbox -> hbox
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox_table), hbox, FALSE, FALSE, 0);

  label = gtk_label_new ("Block size: ");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  block_size_combo = gtk_combo_box_text_new_with_entry ();
  gtk_box_pack_start (GTK_BOX (hbox), block_size_combo, FALSE, FALSE, 0);

  for (i = 0; block_sizes_str[i]; i++)
  {
     gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (block_size_combo),
                                     block_sizes_str[i]);
     if (block_sizes[i] == options.block_size) {
        combo_active = i;
     }
  }

  if (combo_active != -1) {
     gtk_combo_box_set_active (GTK_COMBO_BOX (block_size_combo), combo_active);
  } else {
     char buf[50];
     GtkWidget * entry = gtk_bin_get_child (GTK_BIN (block_size_combo));
     snprintf (buf, sizeof(buf), "%d", options.block_size);
     gtk_entry_set_text (GTK_ENTRY (entry), buf);
  }

  // vbox -> frame -> vbox -> hbox
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 30);
  gtk_box_pack_start (GTK_BOX (vbox_table), hbox, FALSE, FALSE, 0);

  radio = gtk_radio_button_new_with_mnemonic (NULL, "Style _1");
  gtk_box_pack_start (GTK_BOX (hbox), radio, FALSE, FALSE, 0);
  radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
  radio_style1 = radio;

  radio = gtk_radio_button_new_with_mnemonic (radio_group, "Style _2");
  gtk_box_pack_start (GTK_BOX (hbox), radio, FALSE, FALSE, 0);
  radio_style2 = radio;

  if (options.block_style != 1) {
     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_style2), TRUE);
  }

  // -------------
  // vbox -> frame
  frame = gtk_frame_new ("Level");
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  // vbox -> frame -> vbox
  vbox_table = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox_table);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_table), 5);

  labels[0] = gtk_label_new ("Start level: ");
  adjs[0]   = (GtkAdjustment *) gtk_adjustment_new (options.start_level, 0,
                                   NUM_LEVELS-1, 1, 1, 0);
  spins[0]  = gtk_spin_button_new (adjs[0], 0, 0);
  spin_level = spins[0];

  labels[1] = gtk_label_new ("Noise level: ");
  adjs[1]   = (GtkAdjustment *) gtk_adjustment_new (options.noise_level, 0 ,
                                   MAX_X-1, 1, 1, 0);
  spins[1]  = gtk_spin_button_new (adjs[1], 0, 0);
  spin_noise_level = spins[1];

  labels[2] = gtk_label_new ("Noise height: ");
  adjs[2]   = (GtkAdjustment *) gtk_adjustment_new (options.noise_height, 0,
                                   MAX_Y-4, 1, 1, 0);
  spins[2]  = gtk_spin_button_new (adjs[2], 0, 0);
  spin_noise_height = spins[2];

  // vbox -> frame -> vbox -> hbox[x]
  for (i = 0; i < 3; i++)
  {
    hbox_row[i] = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_table), hbox_row[i], FALSE, FALSE, 2);

    gtk_box_pack_end (GTK_BOX (hbox_row[i]), spins[i],  FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbox_row[i]), labels[i], FALSE, FALSE, 0);
  }

  button = gtk_dialog_add_button (GTK_DIALOG (settings_dialog),
                                  "gtk-ok", GTK_RESPONSE_OK);
  button = gtk_dialog_add_button (GTK_DIALOG (settings_dialog),
                                  "gtk-cancel", GTK_RESPONSE_CANCEL);
  gtk_widget_grab_focus (button);

  g_signal_connect (settings_dialog, "response",
                    G_CALLBACK (settings_dialog_response_cb), NULL);

  gtk_widget_show_all (settings_dialog);
}
