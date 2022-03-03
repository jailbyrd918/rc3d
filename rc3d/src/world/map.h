/*
        DESCRIPTION:
                World map definition and handling
*/

#ifndef MAP_H
#define MAP_H


#define TILE_SIZE       (64)

typedef struct {
        char    wall_tex_id;
        char    floor_tex_id;
        int     tile_height;
}
tile_t;

typedef struct {
        char    *id;

        tile_t  *tiles;

        int     rows, cols;
        int     w, h;
}
map_t;

struct {
        map_t   *data;
        size_t  size;
        size_t  capacity;
}
map_list;


void map_init_list
(void);

void map_free_list
(void);


// >> load map data from given file
// >>> [arg] new_map_id: enter the name ID for the map
bool map_add_to_list
(const char *new_map_id, const char *filename);

bool map_remove_from_list
(const char *map_id);

bool map_log_info
(const char *map_id);

bool map_render
(const char *map_id);


map_t *map_get_by_id
(const char *map_id);


int map_get_wall_tex_id
(const char *map_id, const float x, const float y);

int map_get_floor_tex_id
(const char *map_id, const float x, const float y);

int map_get_tile_height
(const char *map_id, const float x, const float y);


#endif
