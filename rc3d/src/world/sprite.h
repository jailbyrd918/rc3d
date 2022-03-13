/*
        DESCRIPTION:
                Sprites (enemies, collectibles, weapons, etc.) definition and handling
*/

#ifndef SPRITE_H
#define SPRITE_H


typedef struct {
        char    *id;

        float   pos_x,
                pos_y,
                pos_z;

        float   dist;
        float   angle;

        char    *tex_id;

        bool    visible;
}
sprite_t;

struct {
        sprite_t        *data;
        size_t          size;
        size_t          capacity;
}
sprite_list;

struct {
        sprite_t        **sprite_refs;
        size_t          size;
        size_t          capacity;
}
visible_sprite_list;


void sprite_init_lists
(void);

void sprite_free_lists
(void);


bool sprite_add_to_list
(const char *new_sprite_id, const char *map_id, const float position_x, const float position_y, const char *texture_id);

bool sprite_remove_from_list
(const char *sprite_id);


bool sprite_check_visible
(void);

bool sprite_render_2d
(const char *map_id);


sprite_t *sprite_get_by_id
(const char *sprite_id);


#endif
