#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include <SDL.h>

#include "utils/str.h"
#include "graphics/display.h"

#include "map.h"


void map_init_list
(void)
{
        map_list.size = 0;
        map_list.capacity = 10;
        map_list.data = malloc(sizeof(map_t) * map_list.capacity);
}

void map_free_list
(void)
{
        for (size_t i = 0; i < map_list.size; ++i)
                free(map_list.data[i].tiles);
        
        free(map_list.data);
}

bool map_add_to_list
(const char *new_map_id, const char *filename)
{
        if (!new_map_id || !filename || str_empty(new_map_id) || str_empty(filename))
                return false;


        // -- create and name new map -- //////////////////

        map_t *newmap = malloc(sizeof(map_t));
        newmap->id = malloc(strlen(new_map_id));
        strcpy(newmap->id, new_map_id);


        // -- read map data from file -- //////////////////

        FILE *mapfile = fopen(filename, "r");

        fseek(mapfile, 0, SEEK_END);
        size_t filesize = ftell(mapfile);
        fseek(mapfile, 0, SEEK_SET);

        char *mapbuf = malloc(filesize + 1);

        fread(mapbuf, sizeof(char), filesize, mapfile);
        mapbuf[filesize] = '\0';
        fclose(mapfile);

        // --------------------------------------------- //


        if (str_empty(mapbuf)) return false;
        int curridx = -1;


        // -- load map rows -- ////////////////////////////

        curridx = search_substr(mapbuf, "map.properties.rows");

        if (curridx == -1)
                return false;
        else {
                while (mapbuf[curridx] != '\0') {
                        if (isdigit(mapbuf[curridx])) {
                                // extract map rows in string format
                                char valstr[64];
                                size_t valstritr = 0;
                                while (mapbuf[curridx] != ' ' && mapbuf[curridx] != '\n')
                                        valstr[valstritr++] = mapbuf[curridx++];
                                valstr[valstritr] = '\0';

                                // convert map rows string to integer value
                                int maprows = -1;
                                sscanf(valstr, "%d", &maprows);

                                newmap->rows = maprows;
                                newmap->w = maprows * TILE_SIZE;
                                break;
                        }

                        ++curridx;
                }
        }


        // -- load map columns -- /////////////////////////

        curridx = search_substr(mapbuf, "map.properties.columns");

        if (curridx == -1)
                return false;
        else {
                while (mapbuf[curridx] != '\0') {
                        if (isdigit(mapbuf[curridx])) {
                                // extract map cols in string format
                                char valstr[64];
                                size_t valstritr = 0;
                                while (mapbuf[curridx] != ' ' && mapbuf[curridx] != '\n')
                                        valstr[valstritr++] = mapbuf[curridx++];
                                valstr[valstritr] = '\0';

                                // convert map cols string to integer value
                                int mapcols = -1;
                                sscanf(valstr, "%d", &mapcols);

                                newmap->cols = mapcols;
                                newmap->h = mapcols * TILE_SIZE;
                                break;
                        }

                        ++curridx;
                }
        }


        // -- allocate tiles array -- /////////////////////

        newmap->tiles = malloc(sizeof(tile_t) * (newmap->rows * newmap->cols));
        int numtiles = newmap->rows * newmap->cols;


        // -- load wall texture code array -- /////////////

        curridx = search_substr(mapbuf, "map.textures.wall");

        if (curridx == -1)
                return false;
        else {
                size_t tileitr = 0;

                while (mapbuf[curridx] != '\0') {
                        if (tileitr == (size_t)numtiles)
                                break;

                        if (isdigit(mapbuf[curridx])) {
                                // extract wall texture id in string format
                                char valstr[64];
                                size_t valstritr = 0;
                                while (mapbuf[curridx] != ' ' && mapbuf[curridx] != '\t' && mapbuf[curridx] != '\n')
                                        valstr[valstritr++] = mapbuf[curridx++];
                                valstr[valstritr] = '\0';

                                // convert wall texture id string to integer value
                                int walltexid = -1;
                                sscanf(valstr, "%d", &walltexid);

                                newmap->tiles[tileitr++].wall_tex_id = walltexid;
                        }

                        ++curridx;
                }
        }


        // -- load floor texture code array -- ////////////

        curridx = search_substr(mapbuf, "map.textures.floor");

        if (curridx == -1)
                return false;
        else {
                size_t tileitr = 0;

                while (mapbuf[curridx] != '\0') {
                        if (tileitr == (size_t)numtiles)
                                break;

                        if (isdigit(mapbuf[curridx])) {
                                // extract wall texture id in string format
                                char valstr[64];
                                size_t valstritr = 0;
                                while (mapbuf[curridx] != ' ' && mapbuf[curridx] != '\t' && mapbuf[curridx] != '\n')
                                        valstr[valstritr++] = mapbuf[curridx++];
                                valstr[valstritr] = '\0';

                                // convert wall texture id string to integer value
                                int floortextid = -1;
                                sscanf(valstr, "%d", &floortextid);

                                newmap->tiles[tileitr++].floor_tex_id = floortextid;
                        }

                        ++curridx;
                }
        }


        // -- load floor height array -- //////////////////

        curridx = search_substr(mapbuf, "map.tile_heights");

        if (curridx == -1)
                return false;
        else {
                size_t tileitr = 0;

                while (mapbuf[curridx] != '\0') {
                        if (tileitr == (size_t)numtiles)
                                break;

                        if (isdigit(mapbuf[curridx]) || mapbuf[curridx] == '-') {
                                // extract wall texture id in string format
                                char valstr[64];
                                size_t valstritr = 0;
                                while (mapbuf[curridx] != ' ' && mapbuf[curridx] != '\t' && mapbuf[curridx] != '\n')
                                        valstr[valstritr++] = mapbuf[curridx++];
                                valstr[valstritr] = '\0';

                                // convert wall texture id string to integer value
                                int tileh = -1;
                                sscanf(valstr, "%d", &tileh);

                                newmap->tiles[tileitr++].tile_height = tileh;
                        }

                        ++curridx;
                }
        }


        // -- resize the map list if full -- //////////////

        if (map_list.size >= map_list.capacity) {
                map_list.capacity *= 2;

                // reallocate map array memory block
                map_t *newmem = realloc(map_list.data, sizeof(map_t) * map_list.capacity);
                if (!newmem) return false;

                map_list.data = newmem;
        }


        // -- add the new map to the list -- //////////////

        map_list.data[map_list.size++] = *newmap;

        free(newmap);
        return true;
}

bool map_remove_from_list
(const char *map_id)
{
        // check if map under given map_id exists in map list
        map_t *rmvmap = map_get_by_id(map_id);
        if (!rmvmap) return false;

        // swap the remove map element to the last element and decrement the size
        // making the illusion of removing the element
        for (size_t i = 0; i < map_list.size; ++i) {
                if (str_matched(map_list.data[i].id, map_id)) {
                        if (i == map_list.size - 1)
                                --map_list.size;
                        else {
                                map_t *lastmap = &(map_list.data[map_list.size - 1]);
                                map_t *temp = malloc(sizeof(map_t));
                                memmove(temp, rmvmap, sizeof(map_t));
                                memmove(rmvmap, lastmap, sizeof(map_t));
                                memmove(lastmap, temp, sizeof(map_t));

                                free(temp);
                                --map_list.size;
                        }

                        break;
                }
        }

        return true;
}


bool map_log_info
(const char *map_id)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        // print ID
        fprintf(stdout, "\nMap ID: %s\n", map->id);

        // print columns
        fprintf(stdout, "Map Columns: %d\n", map->cols);

        // print rows
        fprintf(stdout, "Map Rows: %d\n", map->rows);

        // print wall texture id array
        fprintf(stdout, "\nMap Wall Texture Codes:\n");

        for (int x = 0; x < map->cols; ++x) {
                for (int y = 0; y < map->rows; ++y)
                        fprintf(stdout, "%d ", map->tiles[(map->cols * y) + x].wall_tex_id);

                fprintf(stdout, "\n");
        }

        // print floor texture id array
        fprintf(stdout, "\nMap Floor Texture Codes:\n");

        for (int x = 0; x < map->cols; ++x) {
                for (int y = 0; y < map->rows; ++y)
                        fprintf(stdout, "%d ", map->tiles[(map->cols * y) + x].floor_tex_id);

                fprintf(stdout, "\n");
        }

        // print floor height array
        fprintf(stdout, "\nMap Tile Heights:\n");

        for (int x = 0; x < map->cols; ++x) {
                for (int y = 0; y < map->rows; ++y)
                        fprintf(stdout, "%d\t", map->tiles[(map->cols * y) + x].tile_height);

                fprintf(stdout, "\n");
        }

        return true;
}

bool map_render_2d
(const char *map_id)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        for (int col = 0; col < map->cols; ++col) {
                for (int row = 0; row < map->rows; ++row) {
                        // get the tile screen (x, y) position based on level data index
                        float tilex = (float)(col * TILE_SIZE);
                        float tiley = (float)(row * TILE_SIZE);

                        // determine whether the tile is a wall or empty space
                        int r = map->tiles[(map->cols * row) + col].tile_height == 0 ? 0xff : 0x00;
                        int g = map->tiles[(map->cols * row) + col].tile_height == 0 ? 0xff : 0x00;
                        int b = map->tiles[(map->cols * row) + col].tile_height == 0 ? 0xff : 0x00;
                        int a = (int)(((float)map->tiles[(map->cols * row) + col].tile_height / 0xff) * 0xff);

                        display_draw_rect_f(
                                (tilex * DISPLAY_2D_SCALE) + (g_screenbuf_width / 2) - ((map->w / 2) * DISPLAY_2D_SCALE),
                                (tiley * DISPLAY_2D_SCALE) + (g_screenbuf_height / 2) - ((map->h / 2) * DISPLAY_2D_SCALE),
                                TILE_SIZE * DISPLAY_2D_SCALE,
                                TILE_SIZE * DISPLAY_2D_SCALE,
                                GET_COLOR_HEX(r, g, b, a)
                        );
                }
        }

        return true;
}


map_t *map_get_by_id
(const char *map_id)
{
        if (!map_id || str_empty(map_id))
                return NULL;

        for (size_t i = 0; i < map_list.size; ++i)
                if (str_matched(map_list.data[i].id, map_id))
                        return &(map_list.data[i]);

        return NULL;
}


int map_get_wall_tex_id
(const char *map_id, const float x, const float y)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return -1;

        int col = (int)floorf(x / TILE_SIZE) % map->cols;
        int row = (int)floorf(y / TILE_SIZE) % map->rows;

        return (map->tiles[(map->cols * row) + col].wall_tex_id);
}

int map_get_floor_tex_id
(const char *map_id, const float x, const float y)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return -1;

        int col = (int)floorf(x / TILE_SIZE) % map->cols;
        int row = (int)floorf(y / TILE_SIZE) % map->rows;

        return (map->tiles[(map->cols * row) + col].floor_tex_id);
}

int map_get_tile_height
(const char *map_id, const float x, const float y)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return 0;

        int col = (int)floorf(x / TILE_SIZE) % map->cols;
        int row = (int)floorf(y / TILE_SIZE) % map->rows;

        return (map->tiles[(map->cols * row) + col].tile_height);
}

int map_get_tile_index_x
(const char *map_id, const float x, const float y)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return -1;

        int col = (int)floorf(x / TILE_SIZE) % map->cols;
        return col;
}

int map_get_tile_index_y
(const char *map_id, const float x, const float y)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return -1;

        int row = (int)floorf(y / TILE_SIZE) % map->rows;
        return row;
}

bool map_same_tile
(const char *map_id, const float x0, const float y0, const float x1, const float y1)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return -1;

        int col0 = (int)floorf(x0 / TILE_SIZE) % map->cols;
        int row0 = (int)floorf(y0 / TILE_SIZE) % map->rows;
        int col1 = (int)floorf(x1 / TILE_SIZE) % map->cols;
        int row1 = (int)floorf(y1 / TILE_SIZE) % map->rows;

        return (col0 == col1) && (row0 == row1);
}


