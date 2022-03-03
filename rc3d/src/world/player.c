#include <stdbool.h>

#include <SDL.h>

#include "utils/maths.h"
#include "graphics/display.h"
#include "map.h"

#include "player.h"


void player_init
(const float start_x, const float start_y, const int move_speed, const int turn_speed, const int field_of_view)
{
        // -- translation -- //////////////////////////////////

        player.pos_x = start_x;
        player.pos_y = start_y;
        player.pos_z = 45;
        player.move_dir = 1;
        player.curr_speed = 0.f;
        player.move_speed = move_speed;
        player.moving = false;


        // -- rotation -- /////////////////////////////////////

        player.yaw = math_angle_from_vec2(0.f, -1.f);
        player.yaw_dir = 0;
        player.yaw_speed = (int)DEG_TO_RAD(turn_speed);
        player.fov = DEG_TO_RAD(field_of_view);
}

bool player_update
(const char *map_id, const float delta_time)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        // rotate player
        player.yaw += (player.yaw_dir * player.yaw_speed) * delta_time;
        math_normalize_angle(&player.yaw);

        // move player
        player.curr_speed += (player.moving) ? player.move_speed * delta_time * 2.f : (-player.move_speed) * delta_time * 2.f;
        player.curr_speed = CLAMP(player.curr_speed, 0.f, player.move_speed);

        float movedelta = player.move_dir * player.curr_speed;

        float   targetx = player.pos_x + cosf(player.yaw) * movedelta,
                targety = player.pos_y + sinf(player.yaw) * movedelta;

        bool    insidebound = (player.pos_x > TILE_SIZE) && (player.pos_y > TILE_SIZE) && (player.pos_x < map->w - TILE_SIZE) && (player.pos_x < map->h - TILE_SIZE);

        if (map_get_tile_height(map_id, targetx, targety) <= ((player.pos_z / 2) + map_get_tile_height(map_id, player.pos_x, player.pos_y))) {
                player.pos_x = targetx;
                player.pos_y = targety;
        }

        return true;
}


bool player_render
(const char *map_id)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;


        // -- render player's yaw line -- //////////////////////

        int yawlinelen = 100;

        float endx = player.pos_x + cosf(player.yaw) * yawlinelen;
        float endy = player.pos_y + sinf(player.yaw) * yawlinelen;

        display_draw_line(
                (player.pos_x * DISPLAY_2D_SCALE) + (g_screenbuf_width / 2) - ((map->w / 2) * DISPLAY_2D_SCALE),
                (player.pos_y * DISPLAY_2D_SCALE) + (g_screenbuf_height / 2) - ((map->h / 2) * DISPLAY_2D_SCALE),
                (endx * DISPLAY_2D_SCALE) + (g_screenbuf_width / 2) - ((map->w / 2) * DISPLAY_2D_SCALE),
                (endy * DISPLAY_2D_SCALE) + (g_screenbuf_height / 2) - ((map->h / 2) * DISPLAY_2D_SCALE),
                GET_OPAQUE_COLOR_HEX(0x00, 0x00, 0xff)
        );


        // -- render player rect -- ////////////////////////////

        int playersize = 25;

        display_draw_rect_f(
                ((player.pos_x - (int)floorf((float)playersize / 2.f)) * DISPLAY_2D_SCALE) + (g_screenbuf_width / 2) - ((map->w / 2) * DISPLAY_2D_SCALE),
                ((player.pos_y - (int)floorf((float)playersize / 2.f)) * DISPLAY_2D_SCALE) + (g_screenbuf_height / 2) - ((map->h / 2) * DISPLAY_2D_SCALE),
                playersize * DISPLAY_2D_SCALE,
                playersize * DISPLAY_2D_SCALE,
                GET_OPAQUE_COLOR_HEX(0x00, 0xff, 0x00)
        );

        return true;
}

