/* 
 * draw on surfaces with Cairo 
 * - interface.c: provides cairo context
 * - tetris.c   : calls set_block
 */

#include "tetris.h"

/*
 There are 2 block styles
 1 - use true images... blocks.xpm
 2 - draw block colors using a RGB source surface
     also draw block "borders"

 blocks.xpm contains 8 blocks in a single img - 8 colors
 block 0 = black / background .. ignored
 1.....7 = block colors
*/

static cairo_surface_t * tetris_block[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } ;
static GdkPixbuf * blocks_pixbuf;


// this is for set_block()
//       block_style 2      palette N rgb
static const int block_color_rgb[2][8][3] =
{
   {
      { 0,   0,   0   }, /* 0 black  */
      { 97,  97,  213 }, /* 1 blue   */
      { 97,  209, 98  }, /* 2 green  */
      { 212, 97,  98  }, /* 3 redish */
      { 217, 217, 218 }, /* 4 white  */
      { 212, 97,  213 }, /* 5 purple */
      { 97,  204, 203 }, /* 6 cyan   */
      { 212, 212, 98  }, /* 7 yellow */
   },
};


void set_background_color (cairo_t * cr, GdkRectangle * area)
{
   cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
   cairo_rectangle (cr,
                    area->x, area->y,
                    area->width, area->height);
   cairo_fill (cr);
}


static void
_cairo_resize_surface (cairo_surface_t ** in_surface, int new_width, int new_height)
{
   cairo_t * new_cr;
   cairo_surface_t * out_surface;
   int in_w, in_h;
   double scale_x, scale_y;

   // get current width x height
   in_w = cairo_image_surface_get_width (*in_surface);
   in_h = cairo_image_surface_get_height (*in_surface);

   if (in_w == new_width && in_h == new_height) {
      // no need to resize
      return;
   }

   // create a surface with the desired width x height
   ///cairo_format_t format = cairo_image_surface_get_format (*in_surface);
   ///out_surface = cairo_image_surface_create (format, new_width, new_height);
   out_surface = gdk_window_create_similar_surface (gtk_widget_get_window (game_area),
                                                    CAIRO_CONTENT_COLOR_ALPHA,
                                                    new_width, new_height);
   new_cr  = cairo_create (out_surface);

   // determine scale_x and scale_y
   scale_x = (double) new_width  / (double) in_w;
   scale_y = (double) new_height / (double) in_h;

   // scale and draw on out_surface
   cairo_scale (new_cr, scale_x, scale_y);
   cairo_set_source_surface (new_cr, *in_surface, 0, 0);
   cairo_paint (new_cr);
   cairo_destroy (new_cr);

   // set out_surface as the new surface
   cairo_surface_destroy (*in_surface);
   *in_surface = out_surface;
}


static void //  https://stackoverflow.com/a/36483677 
_cairo_gdk_draw_pixbuf (cairo_t *cr,
                        int src_x,   int src_y,
                        int dest_x,  int dest_y,
                        int width,   int height)
{
   // This a special case where we don't need
   //   the whole pixbuf, just a region.
   // It's a bit complicated with cairo,
   //   so we need the source's surface to perform operations.
   // See
   //   https://developer.gnome.org/gdk2/stable/gdk2-Drawing-Primitives.html#gdk-draw-drawable
   cairo_pattern_t * pattern;
   cairo_surface_t * source;
   pattern = cairo_get_source (cr);
   cairo_pattern_get_surface (pattern, &source);

   cairo_save (cr);

   /* Move (0, 0) to the destination position */
   cairo_translate (cr, dest_x, dest_y);

   /* Set up the source surface in such a way that
    * (src_x, src_y) maps to (0, 0) in user coordinates. */
   cairo_set_source_surface (cr, source, -src_x, -src_y);

   /* Do the drawing */
   cairo_rectangle (cr, 0, 0, width, height);
   cairo_fill (cr);

   /* Undo all of our modifications to the drawing state */
   cairo_restore (cr);
}


void load_tetris_blocks ()
{
   float orig_block_width, orig_block_height;
   float src_x, src_y, dest_x, dest_y, width, height;
   int i;
   cairo_t * cr;
   cairo_format_t format;

   // blocks_pixbuf = gdk_pixbuf_new_from_xpm_data (source_blocks_pix);
   blocks_pixbuf = gdk_pixbuf_new_from_file ("blocks.bmp", NULL);
   orig_block_height = gdk_pixbuf_get_height (blocks_pixbuf);
   orig_block_width  = orig_block_height;
   if (gdk_pixbuf_get_n_channels (blocks_pixbuf) == 3) {
      format = CAIRO_FORMAT_RGB24;
   } else {
      format = CAIRO_FORMAT_ARGB32;
   }

   // allocate memory for the blocks and extract them from the source pix
   for (i = 0; i < 8; i++)
   {
      width = orig_block_width;
      height = orig_block_height;
      src_x = i * width;
      src_y = 0;
      dest_x = 0;
      dest_y = 0;

      tetris_block[i] = cairo_image_surface_create (format, width, height);
      cr = cairo_create (tetris_block[i]);

      gdk_cairo_set_source_pixbuf (cr, blocks_pixbuf,
                                   src_x, src_y);
      _cairo_gdk_draw_pixbuf (cr,
                              src_x,  src_y,
                              dest_x, dest_y,
                              width, height);
      // may need to resize the block
      _cairo_resize_surface (&tetris_block[i], BLOCK_WIDTH, BLOCK_HEIGHT);

      cairo_destroy (cr);
   }
}


void free_tetris_blocks (void)
{
   int i;
   for (i = 0; i < 8; i++)
   {
      if (tetris_block[i]) {
         cairo_surface_destroy (tetris_block[i]);
         tetris_block[i] = NULL;
      }
   }
}


void set_block(int x,int y,int color,int next)
{
   int dest_x, dest_y;
   cairo_t * cr;
   GdkWindow * gdkwin;
   double line_width;
   double r, g, b;
   gboolean draw_grid;

   dest_x = x*BLOCK_WIDTH;
   dest_y = y*BLOCK_HEIGHT;

   if (next) {
      gdkwin = gtk_widget_get_window (next_block_area);
   } else {
      gdkwin = gtk_widget_get_window (game_area);
   }

#if GTK_CHECK_VERSION (3, 22, 0)
   GdkDrawingContext * gdc;
   cairo_region_t * region;
   cairo_rectangle_int_t rect = { 
      .x = x,
      .y = y,
      .width = BLOCK_WIDTH,
      .height = BLOCK_HEIGHT
   };
   region = cairo_region_create_rectangle (&rect);
   gdc = gdk_window_begin_draw_frame (gdkwin, region);
   cairo_region_destroy (region);
   cr = gdk_drawing_context_get_cairo_context (gdc);
#else
   cr = gdk_cairo_create (gdkwin);
#endif

   // need to avoid cairo_set_source_surface() as much as possible
   //   https://stackoverflow.com/questions/15773965/how-to-fast-redrawing-an-image-buffer
   if (color == 0)
   { // black
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
      cairo_rectangle (cr, dest_x, dest_y, BLOCK_WIDTH, BLOCK_HEIGHT);
      cairo_fill (cr);
   } else {
      if (options.block_style == 1) {
         // use blocks from pixbuf
         // draw on target x,y. source already has correct width and height
         cairo_set_source_surface (cr, tetris_block[color], dest_x, dest_y);
         cairo_paint (cr);
      } else {
         // draw rgb blocks
         r = (double) block_color_rgb[0][color][0] / 255.0;
         g = (double) block_color_rgb[0][color][1] / 255.0;
         b = (double) block_color_rgb[0][color][2] / 255.0;
         cairo_set_source_rgb (cr, r, g, b);
         cairo_rectangle (cr, dest_x, dest_y, BLOCK_WIDTH, BLOCK_HEIGHT);
         cairo_fill (cr);
      }
   }

   draw_grid = FALSE;
   if (color == 0 && options.show_grid && !next) {
      // show grid only in game_area
      draw_grid = TRUE;
      cairo_set_source_rgb (cr, 0.2, 0.2, 0.2);
   }
   if (color != 0 && options.block_style != 1) {
      // not exactly a grid, just the block border
      // otherwise there is no block "border"
      draw_grid = TRUE;
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
   }

   if (draw_grid == TRUE)
   {
      // line width 1 = 32x32
      line_width = (double) BLOCK_WIDTH / 32.0;
      cairo_set_line_width (cr, line_width);

      cairo_rectangle (cr,
                       dest_x + line_width,
                       dest_y + line_width,
                       BLOCK_WIDTH - (line_width*2),
                       BLOCK_WIDTH - (line_width*2));
      cairo_stroke (cr);
   }

#if GTK_CHECK_VERSION (3, 22, 0)
   gdk_window_end_draw_frame (gdkwin, gdc);
#else
   cairo_destroy (cr);
#endif
}

