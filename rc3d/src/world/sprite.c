#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>

#include "utils/str.h"
#include "utils/maths.h"
#include "graphics/display.h"
#include "graphics/texture.h"
#include "map.h"
#include "player.h"

#include "sprite.h"


static inline int _compare_sprites_by_distance(const void *sprite1, const void *sprite2) {
        return ((*(sprite_t **)sprite2)->dist < (*(sprite_t **)sprite1)->dist) ? -1 : 1;
}

static sprite_t **_sprite_get_visible_ref_by_id
(const char *sprite_id)
{
        if (!sprite_id || str_empty(sprite_id))
                return NULL;

        for (size_t i = 0; i < visible_sprite_list.size; ++i)
                if (str_matched(visible_sprite_list.sprite_refs[i]->id, sprite_id))
                        return &(visible_sprite_list.sprite_refs[i]);

        return NULL;
}

static bool _sprite_add_visible_to_list
(const char *sprite_id)
{
        if (!sprite_id || str_empty(sprite_id))
                return false;


        // -- if the sprite is already in the list then bail -- //

        for (size_t i = 0; i < visible_sprite_list.size; ++i)
                if (str_matched(visible_sprite_list.sprite_refs[i]->id, sprite_id))
                        return false;


        // -- expand sprites list if full -- //

        if (visible_sprite_list.size >= visible_sprite_list.capacity) {
                visible_sprite_list.capacity *= 2;

                sprite_t **newmem = realloc(visible_sprite_list.sprite_refs, sizeof(sprite_t *) * visible_sprite_list.capacity);
                if (!newmem)     return false;

                visible_sprite_list.sprite_refs = newmem;
        }


        // -- add sprite list to visible sprite list

        sprite_t *sprite = sprite_get_by_id(sprite_id);
        if (!sprite)     return false;

        sprite->visible = true;
        visible_sprite_list.sprite_refs[visible_sprite_list.size++] = sprite;

        return true;
}

static bool _sprite_remove_visible_from_list
(const char *sprite_id)
{
        // check if sprite ref with given sprite_id exists in sprite list
        sprite_t **rmvspriteref = _sprite_get_visible_ref_by_id(sprite_id);
        if (!rmvspriteref)      return false;

        sprite_t *rmvsprite = sprite_get_by_id(sprite_id);
        if (!rmvsprite)         return false;
        rmvsprite->visible = false;

        // swap the impending removal sprite ref element to the last element and decrement the size
        // making the illusion of removing the element
        for (size_t i = 0; i < visible_sprite_list.size; ++i) {
                if (str_matched(visible_sprite_list.sprite_refs[i]->id, sprite_id)) {
                        if (i == visible_sprite_list.size - 1)
                                --visible_sprite_list.size;
                        else {
                                sprite_t **lastspriteref = &(visible_sprite_list.sprite_refs[visible_sprite_list.size - 1]);
                                sprite_t **tempref = malloc(sizeof(sprite_t *));
                                memcpy(tempref, rmvspriteref, sizeof(sprite_t *));
                                memcpy(rmvspriteref, lastspriteref, sizeof(sprite_t *));
                                memcpy(lastspriteref, tempref, sizeof(sprite_t *));

                                free(tempref);
                                --visible_sprite_list.size;
                        }

                        break;
                }
        }

        return true;
}


void sprite_init_lists
(void)
{
        sprite_list.size = 0;
        sprite_list.capacity = 10;
        sprite_list.data = malloc(sizeof(sprite_t) * sprite_list.capacity);

        visible_sprite_list.size = 0;
        visible_sprite_list.capacity = 10;
        visible_sprite_list.sprite_refs = malloc(sizeof(sprite_t *) * visible_sprite_list.capacity);
}

void sprite_free_lists
(void)
{
        sprite_list.size = 0;
        free(sprite_list.data);

        visible_sprite_list.size = 0;
        free(visible_sprite_list.sprite_refs);
}


bool sprite_add_to_list
(const char *new_sprite_id, const char *map_id, const float position_x, const float position_y, const char *texture_id)
{
        if (!new_sprite_id || str_empty(new_sprite_id) || !texture_id || str_empty(texture_id))
                return false;

        if (!map_id || str_empty(map_id))
                return false;

        map_t           *map = map_get_by_id(map_id);

        // create a new sprite
        sprite_t        *newsprite = malloc(sizeof(sprite_t));


        // -- initialize sprite properties -- //

        newsprite->id = malloc(strlen(new_sprite_id));
        strcpy(newsprite->id, new_sprite_id);
        newsprite->pos_x = CLAMP(position_x, 0.f, map->w);
        newsprite->pos_y = CLAMP(position_y, 0.f, map->h);
        newsprite->pos_z = (float)map_get_tile_height(map_id, position_x, position_y);
        newsprite->dist = math_distance_between_points(player.pos_x, player.pos_y, position_x, position_y);
        newsprite->angle = 0.f;
        newsprite->visible = false;
        newsprite->tex_id = malloc(strlen(texture_id));
        strcpy(newsprite->tex_id, texture_id);


        // -- resize sprite list if full -- //

        if (sprite_list.size >= sprite_list.capacity) {
                sprite_list.capacity *= 2;

                sprite_t *newmem = realloc(sprite_list.data, sizeof(sprite_t) * sprite_list.capacity);
                if (!newmem)     return false;

                sprite_list.data = newmem;
        }


        // -- add sprite to list -- //

        sprite_list.data[sprite_list.size++] = *newsprite;

        free(newsprite);
        return true;
}

bool sprite_remove_from_list
(const char *sprite_id)
{
        // check if sprite with given sprite_id exists in sprite list
        sprite_t *rmvsprite = sprite_get_by_id(sprite_id);
        if (!rmvsprite) return false;

        // swap the impending removal sprite element to the last element and decrement the size
        // making the illusion of removing the element
        for (size_t i = 0; i < sprite_list.size; ++i) {
                if (str_matched(sprite_list.data[i].id, sprite_id)) {
                        if (i == sprite_list.size - 1)
                                --sprite_list.size;
                        else {
                                sprite_t *lastsprite = &(sprite_list.data[sprite_list.size - 1]);
                                sprite_t *temp = malloc(sizeof(sprite_t));
                                memmove(temp, rmvsprite, sizeof(sprite_t));
                                memmove(rmvsprite, lastsprite, sizeof(sprite_t));
                                memmove(lastsprite, temp, sizeof(sprite_t));

                                free(temp);
                                --sprite_list.size;
                        }

                        break;
                }
        }

        return true;
}


bool sprite_check_visible(void)
{
        if (sprite_list.size == 0)
                return false;

        for (size_t i = 0; i < sprite_list.size; ++i) {
                sprite_t        *currsprite = &(sprite_list.data[i]);

                // get angle facing player for sprite
                float           angle = player.yaw - atan2f(currsprite->pos_y - player.pos_y, currsprite->pos_x - player.pos_x);

                // clipping epsilon
                // quick fix for the sprite to not be visible when completely out of player view
                float           clipepsilon = .15f;


                // -- restrict the angle to between lefmost and righmost of screen -- //
                // >> leftmost = player_yaw - player_FOV / 2
                // >> rightmost = player_yaw + player_FOV / 2
                if (angle > PI)                 angle -= PI * 2.f;
                else if (angle < PI * -1.f)     angle += PI * 2.f;                
                angle = fabsf(angle);


                // if such angle is within half FOV on either side
                if (angle < (float)player.fov / 2.f + clipepsilon) {
                        currsprite->angle = angle;
                        currsprite->dist = math_distance_between_points(player.pos_x, player.pos_y, currsprite->pos_x, currsprite->pos_y);
                        _sprite_add_visible_to_list(currsprite->id);
                }
                else
                        _sprite_remove_visible_from_list(currsprite->id);
        }

        // sort sprites rendering order by distance
        qsort(visible_sprite_list.sprite_refs, visible_sprite_list.size, sizeof(sprite_t *), _compare_sprites_by_distance);

        return true;
}

bool sprite_render_2d
(const char *map_id)
{
        if (sprite_list.size == 0)
                return false;

        if (!map_id || str_empty(map_id))
                return false;

        map_t           *map = map_get_by_id(map_id);

        uint32_t        spritecolor = 0x00;
        int             spritesize = 50;

        for (size_t i = 0; i < sprite_list.size; ++i) {
                if (sprite_list.data[i].visible)
                        spritecolor = GET_OPAQUE_COLOR_HEX(0x00, 0xff, 0x00);
                else
                        spritecolor = GET_OPAQUE_COLOR_HEX(0xff, 0x00, 0x00);

                display_draw_rect_f(
                        ((sprite_list.data[i].pos_x - (int)floorf((float)spritesize / 2.f)) * DISPLAY_2D_SCALE) + (g_screenbuf_width / 2) - ((map->w / 2) * DISPLAY_2D_SCALE),
                        ((sprite_list.data[i].pos_y - (int)floorf((float)spritesize / 2.f)) * DISPLAY_2D_SCALE) + (g_screenbuf_height / 2) - ((map->h / 2) * DISPLAY_2D_SCALE),
                        spritesize * DISPLAY_2D_SCALE,
                        spritesize * DISPLAY_2D_SCALE,
                        spritecolor
                );
        }

        return true;
}


sprite_t *sprite_get_by_id
(const char *sprite_id)
{
        if (!sprite_id || str_empty(sprite_id))
                return NULL;

        for (size_t i = 0; i < sprite_list.size; ++i)
                if (str_matched(sprite_list.data[i].id, sprite_id))
                        return &(sprite_list.data[i]);

        return NULL;
}

