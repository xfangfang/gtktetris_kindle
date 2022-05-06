/*
 * common includes
 */

#ifndef _TETRIS_H__
#define _TETRIS_H__

#include <gtk/gtk.h>
#include "gtkcompat.h"
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef VERSION
#define VERSION "v0.6.2"
#endif

#ifndef PACKAGE
#define PACKAGE "gtktetris"
#endif

#define MAX_X 10
#define MAX_Y 18
#define do_random(max) ( (int) ((max) * ((float)random() / RAND_MAX)) )

extern GtkApplication * gtktetris_app;
extern int BLOCK_WIDTH;
extern int BLOCK_HEIGHT;
extern GtkWidget * main_window;

#define NUM_LEVELS 20
#define NUM_HIGHSCORE 10

#define START_LEVEL_VALUE 6 // Iavor

#define DEFAULT_USER_NAME "kindle"

// global variables
GtkWidget *game_area;
GtkWidget *next_block_area;
int game_over;
int game_pause;
int current_x;
int current_y;
int current_block;
int current_frame;
long current_score;
int current_level;
int current_lines;
int next_block;
int next_frame;

// main.c
char * get_config_dir_file (const char * file);
void gtktetris_exit (void);

// help.c
void show_help (void);

// options.c
void options_defaults (void);
void options_read(void);
void options_show_dialog (void);

struct Options
{
  int start_level;
  int noise_level;
  int noise_height;
  int show_next_block;
  int show_grid;
  int block_size;
  int block_style;
};

struct Options options;

// tetris.c
void draw_block(int x,int y,int block,int frame,int clear,int next);
void to_virtual();
void from_virtual();
int valid_position(int x,int y,int block,int frame);
void move_block(int x,int y,int f);
int check_lines();
int move_down();
void new_block();
void game_over_clean();
void game_init();
void make_noise(int level,int height);

// interface.c
void create_main_window (void);
void update_block_size (int startup);
void update_game_values();
int game_loop();
void game_over_init();
void game_set_pause();

// draw.c
void load_tetris_blocks ();
void free_tetris_blocks (void);
void set_block(int x,int y,int color,int next);
void set_background_color (cairo_t * cr, GdkRectangle * area);

// highscore.c
void read_highscore();
void write_highscore();
void show_highscore_dlg();
int addto_highscore(char *name,long score, int level, int lines);
long get_hiscore (void);

#endif

