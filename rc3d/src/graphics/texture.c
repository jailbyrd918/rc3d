#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>

#include "utils/str.h"
#include "graphics/display.h"

#include "texture.h"


void texture_init_list
(void)
{
        texture_list.size = 0;
        texture_list.capacity = 10;
        texture_list.data = malloc(sizeof(texture_t) * texture_list.capacity);
}

void texture_free_list
(void)
{
        free(texture_list.data);
}


bool texture_add_to_list
(const char *new_texture_id, const char *filename)
{
        if (!new_texture_id || str_matched(new_texture_id, ""))
                return false;

        if (!filename || str_matched(filename, ""))
                return false;

        // create new texture
        texture_t *newtex = malloc(sizeof(texture_t));

        // create a structure containing collection of pixels 
        SDL_Surface *surface = IMG_Load(filename);
        SDL_PixelFormat *surfacefmt = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
        surface = SDL_ConvertSurface(surface, surfacefmt, 0);
        SDL_FreeFormat(surfacefmt);

        // -- initialize texture properties -- //////////

        newtex->id = malloc(strlen(new_texture_id));
        strcpy(newtex->id, new_texture_id);
        newtex->w = surface->w;
        newtex->h = surface->h;
        newtex->buffer = malloc(sizeof(uint32_t) * (surface->w * surface->h));
        memcpy(newtex->buffer, surface->pixels, sizeof(uint32_t) * (surface->w * surface->h));


        // -- resize texture list if full -- ////////////

        if (texture_list.size >= texture_list.capacity) {
                texture_list.capacity *= 2;

                // reallocate texture array memory block
                texture_t *newmem = realloc(texture_list.data, sizeof(texture_t) * texture_list.capacity);
                if (!newmem) return false;

                texture_list.data = newmem;
        }


        // -- add new texture to list -- ////////////////

        texture_list.data[texture_list.size++] = *newtex;


        SDL_FreeSurface(surface);
        free(newtex);
        return true;
}

bool texture_remove_from_list
(const char *texture_id)
{
        // check if texture under given texture_id exists in texture list
        texture_t *rmvtex = texture_get_by_id(texture_id);
        if (!rmvtex) return false;

        // swap the remove texture element to the last element and decrement the size
        // making the illusion of removing the element
        for (size_t i = 0; i < texture_list.size; ++i) {
                if (str_matched(texture_list.data[i].id, texture_id)) {
                        if (i == texture_list.size - 1)
                                --texture_list.size;
                        else {
                                texture_t *lasttex = &(texture_list.data[texture_list.size - 1]);
                                texture_t *temp = malloc(sizeof(texture_t));
                                memmove(temp, rmvtex, sizeof(texture_t));
                                memmove(rmvtex, lasttex, sizeof(texture_t));
                                memmove(lasttex, temp, sizeof(texture_t));

                                free(temp);
                                --texture_list.size;
                        }

                        break;
                }
        }

        return true;
}


bool texture_draw
(const char *texture_id, const int x, const int y)
{
        texture_t *tex = texture_get_by_id(texture_id);
        if (!tex) return false;

        for (int drawx = 0; drawx < tex->w; ++drawx) {
                for (int drawy = 0; drawy < tex->h; ++drawy) {
                        uint32_t texcolor = tex->buffer[(tex->w * drawy) + drawx];
                        display_draw_pixel(x + drawx, y + drawy, texcolor);
                }
        }

        return true;
}


texture_t *texture_get_by_id
(const char *texture_id)
{
        if (!texture_id || str_matched(texture_id, ""))
                return NULL;

        for (size_t i = 0; i < texture_list.size; ++i)
                if (str_matched(texture_list.data[i].id, texture_id))
                        return &(texture_list.data[i]);

        return NULL;
}

