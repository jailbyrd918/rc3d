/*
        DESCRIPTION:
                Texture definition and handling
*/

#ifndef TEXTURE_H
#define TEXTURE_H


typedef struct {
        char            *id;
        uint32_t        *buffer;
        int             w, h;

        int             frame_w;
        int             frames;
        size_t          frame_index;

        bool            is_anim;
        float           frame_duration;
        float           frame_time;
}
texture_t;

struct {
        texture_t       *data;
        size_t          size;
        size_t          capacity;
}
texture_list;


void texture_init_list
(void);

void texture_free_list
(void);


bool texture_add_to_list
(const char *new_texture_id, const char *filename, const int frame_width, const int frames, const float frame_duration);

bool texture_remove_from_list
(const char *texture_id);


bool texture_draw
(const char *texture_id, const int x, const int y);

void texture_update_all_anims
(const float delta_time);


texture_t *texture_get_by_id
(const char *texture_id);



#endif