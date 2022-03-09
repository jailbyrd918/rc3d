#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>

#include "utils/maths.h"
#include "display.h"
#include "texture.h"
#include "world/map.h"
#include "world/player.h"
#include "world/ray.h"

#include "proj.h"


bool projection_3d_implement
(const char *map_id)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        texture_t       *floortex, *walltex, *skytex;

        int     umost = 0,
                dmost = g_screenbuf_height - 1;

        // horizon row
        int     horizon = (g_screenbuf_height >> 1);

        // distance to projection plane
        float   distproj = (float)horizon / tanf(player.fov / 2);

        // height of the tile at player position
        int     currh = map_get_tile_height(map_id, player.pos_x, player.pos_y);

        for (int x = 0; x < g_screenbuf_width; ++x) {
                float           rayangle = ray_list[x].angle;
                ray_hit_list_t  hitlist = ray_list[x].hit_results;


                // -- draw tracking properties -- //

                int     prevtopy = dmost;
                int     floorstarty = dmost;
                int     floorendy = dmost;


                for (size_t i = 0; i < hitlist.size; ++i) {
                        ray_hit_t       currres = hitlist.data[i];


                        // -- current hit result properties -- //

                        float           currdist = currres.dist;
                        float           currhitx = currres.hitpt_x;
                        float           currhity = currres.hitpt_y;
                        bool            currhitvert = currres.hit_vert_side;
                        int             currwalltexid = currres.wall_tex_id - 1;
                        int             currfloortexid = currres.floor_tex_id - 1;
                        int             currtileh = currres.tile_height;
                        bool            currbackface = currres.back_face;


                        // correct hit result distance to rectify fish-eye effect
                        currdist = currdist * cosf(rayangle - player.yaw);

                        // projected hit result height
                        int             currresh = (int)((currtileh / currdist) * distproj);

                        // hit result top and bottom row
                        int             currbtmy = horizon + (int)(player.pos_z / (currdist / distproj));
                        int             currtopy = ((currbtmy - currresh) < umost) ? umost : (currbtmy - currresh);

                        // -- draw front face -- //

                        if (!currbackface) {
                                walltex = &(texture_list.data[currwalltexid]);
                                int             currwalltexoffsetx = currhitvert ? (int)currhity % walltex->w : (int)currhitx % walltex->w;
                                int             frontdrawy = (currbtmy > prevtopy) ? prevtopy : currbtmy;

                                if (currtopy < prevtopy) {
                                        for (; (frontdrawy >= currtopy) && (frontdrawy > umost); --frontdrawy) {
                                                int     distcurrtop = frontdrawy + currresh - horizon;
                                                float   currwalltexscale = (float)walltex->h / currresh;
                                                int     currwalltexoffsety = (int)(distcurrtop * currwalltexscale) % walltex->h;

                                                uint32_t currestexcolor = walltex->buffer[(walltex->w * currwalltexoffsety) + currwalltexoffsetx];
                                                screenbuf[(g_screenbuf_width * frontdrawy) + x] = currestexcolor;
                                        }

                                        // update face result top render row
                                        prevtopy = currtopy;
                                }
                        }

                        // -- draw top side for hit results -- //
                        else {
                                if (currtopy < prevtopy) {
                                        if (i > 0) {
                                                ray_hit_t       prevres = hitlist.data[i - 1];
                                                float           prevhitx = prevres.hitpt_x;
                                                float           prevhity = prevres.hitpt_y;
                                                bool            prevbackface = prevres.back_face;

                                                if (currbackface && !prevbackface && map_same_tile(map_id, currhitx, currhity, prevhitx, prevhity)) {
                                                        for (int topsidedrawy = prevtopy; (topsidedrawy >= currtopy) && (topsidedrawy > umost); --topsidedrawy) {
                                                                // distance from current draw row to horizon draw
                                                                int     disthorizon = topsidedrawy + (currresh)-horizon;

                                                                // straight distance to floor intersection
                                                                float   distfloorstraight = (player.pos_z / (float)disthorizon) * distproj;

                                                                // actual distance to floor intersection
                                                                float   distflooractual = distfloorstraight / cosf(rayangle - player.yaw);

                                                                float   floorx = player.pos_x + cosf(rayangle) * distflooractual,
                                                                        floory = player.pos_y + sinf(rayangle) * distflooractual;

                                                                if ((floorx > 0) && (floory > 0) && (floorx < map->w) && (floory < map->h)) {

                                                                        int     floortexid = map_get_floor_tex_id(map_id, floorx, floory) - 1;
                                                                        floortex = &(texture_list.data[floortexid]);

                                                                        int     floortexoffsetx = (int)floorx % floortex->w,
                                                                                floortexoffsety = (int)floory % floortex->h;

                                                                        uint32_t floortexcolor = floortex->buffer[(floortex->w * floortexoffsety) + floortexoffsetx];
                                                                        screenbuf[(g_screenbuf_width * topsidedrawy) + x] = floortexcolor;
                                                                }
                                                        }

                                                        // update floor drawing start row
                                                        floorstarty = currtopy;
                                                }
                                                else {
                                                        for (int topsidedrawy = prevtopy; (topsidedrawy >= currtopy) && (topsidedrawy > umost); --topsidedrawy) {
                                                                screenbuf[(g_screenbuf_width * topsidedrawy) + x] = GET_OPAQUE_COLOR_HEX(0x00, 0x00, 0x00);
                                                        }
                                                }
                                        }


                                        // update face result top render row
                                        prevtopy = currtopy;
                                }
                        }


                        // update floor drawing end row
                        floorendy = currbtmy;


                        // -- draw floor between previous hit result top and current hit result bottom -- //

                        if (floorstarty > floorendy) {
                                for (int floorbackdrawy = floorstarty; floorbackdrawy >= floorendy; --floorbackdrawy) {
                                        // distance from current draw row to horizon draw
                                        int     disthorizon = floorbackdrawy - horizon;

                                        // straight distance to floor intersection
                                        float   distfloorstraight = (player.pos_z / (float)disthorizon) * distproj;

                                        // actual distance to floor intersection
                                        float   distflooractual = distfloorstraight / cosf(rayangle - player.yaw);

                                        float   floorx = player.pos_x + cosf(rayangle) * distflooractual,
                                                floory = player.pos_y + sinf(rayangle) * distflooractual;

                                        if ((floorx > 0) && (floory > 0) && (floorx < map->w) && (floory < map->h)) {
                                                int     floortexid = map_get_floor_tex_id(map_id, floorx, floory) - 1;
                                                floortex = &(texture_list.data[floortexid]);

                                                int     floortexoffsetx = (int)floorx % floortex->w,
                                                        floortexoffsety = (int)floory % floortex->h;

                                                uint32_t floortexcolor = floortex->buffer[(floortex->w * floortexoffsety) + floortexoffsetx];
                                                screenbuf[(g_screenbuf_width * floorbackdrawy) + x] = floortexcolor;


                                        }
                                }
                        }


                        // -- draw ceiling / sky -- //

                        skytex = texture_get_by_id("sky_nightcity");

                        for (int skydrawy = umost; (skydrawy < prevtopy) && (skydrawy < (horizon + currh)); ++skydrawy) {
                                int             skytexoffsety = (int)floorf((float)skytex->h / 3.f);
                                int             skyx = (int)floorf((rayangle * skytex->w) / (PI * 2.f)) % skytex->w;
                                uint32_t        skytexcolor = skytex->buffer[(skytex->w * (skydrawy + skytexoffsety)) + skyx];
                                screenbuf[(g_screenbuf_width * skydrawy) + x] = skytexcolor;
                        }


                        // update floor drawing start row
                        floorstarty = prevtopy;
                }


        }

        return true;
}

