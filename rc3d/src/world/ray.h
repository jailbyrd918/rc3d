/*
        DESCRIPTION:
                Ray-casting results definition and handling
*/

#ifndef RAY_H
#define RAY_H


typedef struct {
        float   hitpt_x;
        float   hitpt_y;
        float   dist;

        int     wall_tex_id;
        int     floor_tex_id;
        int     tile_height;

        bool    hit_vert_side;
        bool    back_face;
        bool    bound_tile;
}
ray_hit_t;

typedef struct {
        ray_hit_t       *data;
        size_t          size;
        size_t          capacity;
}
ray_hit_list_t;


typedef struct {
        float                   angle;
        ray_hit_list_t          hit_results;
}
ray_t;

ray_t *ray_list;


void ray_init_list
(void);

void ray_free_list
(void);


bool ray_cast_all
(const char *map_id);

bool ray_render_all
(const char *map_id);


#endif