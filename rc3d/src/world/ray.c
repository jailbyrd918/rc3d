#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>

#include "utils/maths.h"
#include "graphics/display.h"
#include "player.h"
#include "map.h"

#include "ray.h"


static int _compare_rayhit_by_distance
(const ray_hit_t *rayhit1, const ray_hit_t *rayhit2)
{
        return (*(float *)&rayhit1->dist < *(float *)&rayhit2->dist) ? -1 : 1;
}

static bool _add_ray_hit_to_list
(const char *map_id, const int ray_id, const float ray_angle, float hitptx, float hitpty, const bool hit_vert, const bool back_face)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        ray_hit_t rayhit = (ray_hit_t) { 0 };
        rayhit.hitpt_x = hitptx;
        rayhit.hitpt_y = hitpty;
        rayhit.dist = math_distance_between_points(player.pos_x, player.pos_y, hitptx, hitpty);
        rayhit.hit_vert_side = hit_vert;
        rayhit.wall_tex_id = map_get_wall_tex_id(map_id, hitptx, hitpty);
        rayhit.floor_tex_id = map_get_floor_tex_id(map_id, hitptx, hitpty);
        rayhit.tile_height = map_get_tile_height(map_id, hitptx, hitpty);
        rayhit.bound_tile = (hitptx < TILE_SIZE) || (hitpty < TILE_SIZE) || (hitptx >= map->w - TILE_SIZE) || (hitpty >= map->h - TILE_SIZE);
        rayhit.back_face = back_face;

        ray_hit_list_t *hit_results = &(ray_list[ray_id].hit_results);
        if (hit_results->size >= hit_results->capacity) {
                hit_results->capacity *= 2;
                ray_hit_t *newmem = realloc(hit_results->data, sizeof(ray_hit_t) * hit_results->capacity);
                if (!newmem)    return false;
                hit_results->data = newmem;
        }

        hit_results->data[hit_results->size++] = rayhit;
        return true;
}

static void _clear_all_hits
(const int ray_id)
{
        ray_list[ray_id].hit_results.size = 0;
}

static void _ray_cast
(const char *map_id, const int ray_id, float ray_angle)
{
        map_t *map = map_get_by_id(map_id);

        math_normalize_angle(&ray_angle);
        _clear_all_hits(ray_id);

        ray_hit_list_t *ray_hit_list = &(ray_list[ray_id].hit_results);

        // -- ray needs to know which direction the player is facing -- //

        bool    facingdown = (ray_angle > 0) && (ray_angle < PI);
        bool    facingup = !facingdown;
        bool    facingleft = (ray_angle > (PI * .5f) && ray_angle < (PI * 1.5f));
        bool    facingright = !facingleft;


        // -- check horizontal intersections -- //////////////////////////

        // distance to next horizontal intersection
        float   h_stepx = 0.f,
                h_stepy = 0.f;

        // iterated grid intersected hit locations
        float   hfront_hitptx = 0.f,
                hfront_hitpty = 0.f,
                hback_hitptx = 0.f,
                hback_hitpty = 0.f;

        // get initial horizontal intersections at y
        hfront_hitpty = floorf(player.pos_y / TILE_SIZE) * TILE_SIZE;
        hfront_hitpty += (facingdown) ? TILE_SIZE : 0.f;
        hback_hitpty = hfront_hitpty;

        // get initial horizontal intersections at x
        hfront_hitptx = player.pos_x + ((hfront_hitpty - player.pos_y) / tanf(ray_angle));
        hback_hitptx = hfront_hitptx;

        // calculate distance to next horizontal intersection at y
        h_stepy = TILE_SIZE;
        h_stepy *= ((facingup && h_stepy > 0.f) || (facingdown && h_stepy < 0.f)) ? -1.f : 1.f;

        // calculate distance to next horizontal intersection at x
        h_stepx = h_stepy / tanf(ray_angle);
        h_stepx *= ((facingleft && h_stepx > 0.f) || (facingright && h_stepx < 0.f)) ? -1.f : 1.f;

        while (hfront_hitptx > 0.f && hfront_hitptx < (float)map->w && hfront_hitpty > 0.f && hfront_hitpty < (float)map->h) {
                if (map_get_tile_height(map_id, hfront_hitptx, hfront_hitpty - (facingup ? 1.f : 0.f)) > 0) {
                        _add_ray_hit_to_list(map_id, ray_id, ray_angle, hfront_hitptx, hfront_hitpty - (facingup ? 1.f : 0.f), false, false);
                }

                if (map_get_tile_height(map_id, hback_hitptx, hback_hitpty - (facingup ? 0.f : 1.f)) > 0) {
                        _add_ray_hit_to_list(map_id, ray_id, ray_angle, hback_hitptx, hback_hitpty - (facingup ? 0.f : 1.f), false, true);
                }

                hfront_hitptx += h_stepx;
                hfront_hitpty += h_stepy;
                hback_hitptx = hfront_hitptx;
                hback_hitpty = hfront_hitpty;
        }

        // -- check vertical intersection -- /////////////////////////////

        // distance to next horizontal intersection
        float   v_stepx = 0.f,
                v_stepy = 0.f;

        // iterated grid intersected hit location
        float   vfront_hitptx = 0.f,
                vfront_hitpty = 0.f,
                vback_hitptx = 0.f,
                vback_hitpty = 0.f;

        // get initial vertical intersection at x
        vfront_hitptx = floorf(player.pos_x / TILE_SIZE) * TILE_SIZE;
        vfront_hitptx += (facingright) ? TILE_SIZE : 0.f;
        vback_hitptx = vfront_hitptx;

        // get initial vertical intersection at y
        vfront_hitpty = player.pos_y + ((vfront_hitptx - player.pos_x) * tanf(ray_angle));
        vback_hitpty = vfront_hitpty;

        // calculate distance to next vertical intersection at x
        v_stepx = TILE_SIZE;
        v_stepx *= ((facingleft && v_stepx > 0.f) || (facingright && v_stepx < 0.f)) ? -1.f : 1.f;

        // calculate distance to next vertical intersection at y
        v_stepy = v_stepx * tanf(ray_angle);
        v_stepy *= ((facingup && v_stepy > 0.f) || (facingdown && v_stepy < 0.f)) ? -1.f : 1.f;

        while (vfront_hitptx > 0.f && vfront_hitptx < (float)map->w && vfront_hitpty > 0.f && vfront_hitpty < (float)map->h) {
                if (map_get_tile_height(map_id, vfront_hitptx - (facingleft ? 1.f : 0.f), vfront_hitpty) > 0) {
                        _add_ray_hit_to_list(map_id, ray_id, ray_angle, vfront_hitptx - (facingleft ? 1.f : 0.f), vfront_hitpty, true, false);
                }

                if (map_get_tile_height(map_id, vback_hitptx - (facingleft ? 0.f : 1.f), vback_hitpty) > 0) {
                        _add_ray_hit_to_list(map_id, ray_id, ray_angle, vback_hitptx - (facingleft ? 0.f : 1.f), vback_hitpty, true, true);
                }

                vfront_hitptx += v_stepx;
                vfront_hitpty += v_stepy;
                vback_hitptx = vfront_hitptx;
                vback_hitpty = vfront_hitpty;

        }

        // -- record ray angle -- //

        ray_list[ray_id].angle = ray_angle;


        // -- SORT THE RAY HIT RESULT LISTS -- ///////////////////////////

        ray_hit_list_t *hit_results = &(ray_list[ray_id].hit_results);
        qsort(hit_results->data, hit_results->size, sizeof(ray_hit_t), _compare_rayhit_by_distance);

        ray_hit_t hitres;

        for (size_t i = 0; i < hit_results->size; ++i)
                hitres = hit_results->data[i];

        // remove all boundary type ray hits, except for the closest one
        if (hit_results->size > 1) {
                for (size_t i = hit_results->size - 1; i > 0; --i) {
                        if (hit_results->data[i].bound_tile && hit_results->data[i - 1].bound_tile)
                                --hit_results->size;
                }
        }

}


void ray_init_list
(void)
{
        ray_list = malloc(sizeof(ray_t) * g_screenbuf_width);

        for (size_t i = 0; i < (size_t)g_screenbuf_width; ++i) {
                ray_list[i].hit_results.size = 0;
                ray_list[i].hit_results.capacity = 5;
                ray_list[i].hit_results.data = malloc(sizeof(ray_hit_t) * ray_list[i].hit_results.capacity);
        }
}

void ray_free_list
(void)
{
        for (size_t i = 0; i < (size_t)g_screenbuf_width; ++i) {
                free(ray_list[i].hit_results.data);
        }

        free(ray_list);
}


bool ray_cast_all
(const char *map_id)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        // initial ray casting angle
        float rayangle = player.yaw - (player.fov / 2);

        for (size_t rayid = 0; rayid < (size_t)g_screenbuf_width; ++rayid) {
                _ray_cast(map_id, rayid, rayangle);
                rayangle += (player.fov / g_screenbuf_width);
        }

        return true;
}

bool ray_render_all
(const char *map_id)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        for (size_t rayid = 0; rayid < (size_t)g_screenbuf_width; ++rayid) {
                ray_hit_list_t hit_res = ray_list[rayid].hit_results;
                ray_hit_t res = hit_res.data[hit_res.size - 1];

                float   x1 = res.hitpt_x,
                        y1 = res.hitpt_y;

                display_draw_line(
                        (player.pos_x * DISPLAY_2D_SCALE) + (g_screenbuf_width / 2) - ((map->w / 2) * DISPLAY_2D_SCALE),
                        (player.pos_y * DISPLAY_2D_SCALE) + (g_screenbuf_height / 2) - ((map->h / 2) * DISPLAY_2D_SCALE),
                        (x1 * DISPLAY_2D_SCALE) + (g_screenbuf_width / 2) - ((map->w / 2) * DISPLAY_2D_SCALE),
                        (y1 * DISPLAY_2D_SCALE) + (g_screenbuf_height / 2) - ((map->h / 2) * DISPLAY_2D_SCALE),
                        GET_OPAQUE_COLOR_HEX(0xff, 0x00, 0x00)
                );
        }

        return true;
}

