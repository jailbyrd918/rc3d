/*
        DESCRIPTION:
                Handles drawing/rendering or any graphics related functions
*/

#ifndef DISPLAY_H
#define DISPLAY_H


extern	SDL_Window	*window;
extern	SDL_Renderer	*renderer;
extern  int             g_window_width;
extern  int             g_window_height;

extern	uint32_t	*screenbuf;
extern	SDL_Texture	*screenbuf_texture;
extern  int             g_screenbuf_width;
extern  int             g_screenbuf_height;


enum {
        DISPLAY_MODE_2D,
        DISPLAY_MODE_3D,
        DISPLAY_MODES
}
display_mode;

#define DISPLAY_2D_SCALE        (.1f)


#define GET_COLOR_HEX(r, g, b, a)	\
        ((a << 24) + (r << 16) + (g << 8) + (b << 0))

#define GET_OPAQUE_COLOR_HEX(r, g, b)	\
        ((0xff << 24) + (r << 16) + (g << 8) + (b << 0))


bool display_init
(const char *win_title, const int win_w, const int win_h, const int scrbuf_w, const int scrbuf_h);

void display_free
(void);


bool display_clear_screenbuf
(const uint32_t color);

bool display_draw_pixel
(const int x, const int y, const uint32_t color);

bool display_draw_vert
(const int x, const int y0, const int y1, const uint32_t color);

bool display_draw_line
(const float x0, const float y0, const float x1, const float y1, const uint32_t color);

bool display_draw_rect
(const int x, const int y, const int w, const int h, const uint32_t color);

bool display_draw_rect_f
(const float x, const float y, const float w, const float h, const uint32_t color);

bool display_render_screenbuf
(void);


void display_toggle_mode
(void);


#endif
