#include <stdbool.h>

#include <SDL.h>

#include "display.h"


// >> console window
SDL_Window      *window = NULL;

// >> rendering context for the window
SDL_Renderer    *renderer = NULL;

// >> global window width and height
int             g_window_width = 0, g_window_height = 0;


// >> screen pixels buffer
uint32_t        *screenbuf = NULL;

// >> texture rendering screen pixels buffer to screen
SDL_Texture     *screenbuf_texture = NULL;

// >> global screen pixels buffer width and height
int             g_screenbuf_width = 0, g_screenbuf_height = 0;


bool display_init
(const char *win_title, const int win_w, const int win_h, const int scrbuf_w, const int scrbuf_h)
{
        if (win_w <= 0 || win_h <= 0 || scrbuf_w <= 0 || scrbuf_h <= 0)
                return false;

        // create console window
        window = SDL_CreateWindow(win_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, win_w, win_h, SDL_WINDOW_SHOWN);
        if (!window) return false;

        // initialize global window width and height
        SDL_GetWindowSize(window, &g_window_width, &g_window_height);

        // create window rendering context
        renderer = SDL_CreateRenderer(window, -1, 0);
        if (!renderer) return false;

        // allocate screen pixels buffer
        screenbuf = malloc(sizeof(uint32_t) * (scrbuf_w * scrbuf_h));
        if (!screenbuf) return false;

        // initialize global screen buffer width and height
        g_screenbuf_width = scrbuf_w, g_screenbuf_height = scrbuf_h;

        // create screen buffer texture
        screenbuf_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, scrbuf_w, scrbuf_h);
        if (!screenbuf_texture) return false;

        return true;
}

void display_free
(void)
{
        // destroy screen buffer texture
        SDL_DestroyTexture(screenbuf_texture);

        // deallocate screen pixels buffer
        free(screenbuf);

        // destroy console window and its rendering context
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
}


bool display_clear_screenbuf
(const uint32_t color)
{
        if (!screenbuf) return false;

        for (int x = 0; x < g_screenbuf_width; ++x)
                for (int y = 0; y < g_screenbuf_height; ++y)
                        screenbuf[(g_screenbuf_width * y) + x] = color;

        return true;
}

bool display_draw_pixel
(const int x, const int y, const uint32_t color)
{
        if (!screenbuf) return false;
        if (x < 0 || x > g_screenbuf_width || y < 0 || y > g_screenbuf_width)
                return false;

        screenbuf[(g_screenbuf_width * y) + x] = color;

        return true;
}

bool display_draw_vert
(const int x, const int y0, const int y1, const uint32_t color)
{
        if (!screenbuf) return false;

        for (int y = y0; y < y1; ++y)
                screenbuf[(g_screenbuf_width * y) + x] = color;

        return true;
}

bool display_draw_line
(const float x0, const float y0, const float x1, const float y1, const uint32_t color)
{
        if (!screenbuf) return false;
        if (x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0) return false;
        if (x0 > g_screenbuf_width || y0 > g_screenbuf_height || x1 > g_screenbuf_width || y1 > g_screenbuf_height)
                return false;

        // line delta x and y
        float   dx = x1 - x0,
                dy = y1 - y0;

        // get the larger of two deltas
        float   largeside = fabsf(dx) >= fabsf(dy) ? fabsf(dx) : fabsf(dy);

        // calculate line increment steps
        float   xstep = dx / largeside,
                ystep = dy / largeside;

        // draw position (x, y) (incremented by step after drawn)
        float   drawx = x0,
                drawy = y0;

        // draw pixel and increment towards end point (x1, y1)
        for (int i = 0; i < (int)largeside; ++i) {
                if (!display_draw_pixel((int)roundf(drawx), (int)roundf(drawy), color))
                        return false;

                drawx += xstep, drawy += ystep;
        }

        return true;
}

bool display_draw_rect
(const int x, const int y, const int w, const int h, const uint32_t color)
{
        if (!screenbuf) return false;
        if (x < 0 || y < 0 || x > g_screenbuf_width || y > g_screenbuf_height)
                return false;

        for (int currx = x; currx < x + w; ++currx)
                for (int curry = y; curry < y + h; ++curry)
                        screenbuf[(g_screenbuf_width * curry) + currx] = color;

        return true;
}

bool display_draw_rect_f
(const float x, const float y, const float w, const float h, const uint32_t color)
{
        if (!screenbuf) return false;
        if (x < 0.f || y < 0.f || x >(float)g_screenbuf_width || y >(float)g_screenbuf_height)
                return false;

        for (float currx = x; currx < x + w; ++currx)
                for (float curry = y; curry < y + h; ++curry)
                        screenbuf[(g_screenbuf_width * (int)curry) + (int)currx] = color;

        return true;
}

bool display_render_screenbuf
(void)
{
        if (!screenbuf) return false;

        // convert screen pixels buffer data to texture format
        SDL_UpdateTexture(screenbuf_texture, NULL, screenbuf, sizeof(uint32_t) * g_screenbuf_width);

        // render the texture
        SDL_RenderCopy(renderer, screenbuf_texture, NULL, NULL);

        return true;
}

void display_toggle_mode
(void)
{
        switch (display_mode) {
                case DISPLAY_MODE_2D:
                        display_mode = DISPLAY_MODE_3D;
                        break;

                case DISPLAY_MODE_3D:
                        display_mode = DISPLAY_MODE_2D;
                        break;

                default:
                        break;
        }
}
