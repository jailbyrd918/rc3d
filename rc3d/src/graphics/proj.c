#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>

#include "utils/str.h"
#include "utils/maths.h"
#include "display.h"
#include "texture.h"
#include "world/map.h"
#include "world/player.h"
#include "world/ray.h"
#include "world/sprite.h"

#include "proj.h"


bool projection_3d_implement
(const char *map_id, const bool toggle_sky, const char *sky_tex_id)
{
        map_t *map = map_get_by_id(map_id);
        if (!map) return false;

        int     umost = 0,
                dmost = g_screenbuf_height - 1;

        // horizon row
        int     horizon = (g_screenbuf_height >> 1);

        // distance to projection plane
        float   distproj = (float)horizon / tanf(player.fov / 2);


        // -- iterate ray by ray -- //

        for (int x = 0; x < g_screenbuf_width; ++x) {
                float           rayangle = ray_list[x].angle;
                ray_hit_list_t  rayhits = ray_list[x].hit_results;


                // -- draw tracking properties -- //

                int     drawnmosty = dmost;
                int     floorstarty = dmost;
                int     floorendy = dmost;


                for (size_t i = 0; i < rayhits.size; ++i) {
                        ray_hit_t       currhit = rayhits.data[i];


                        // -- current ray hit result properties -- //

                        float           currhitdist = currhit.dist;
                        float           currhitx = currhit.hitpt_x;
                        float           currhity = currhit.hitpt_y;
                        int             currhitwalltexid = currhit.wall_tex_id - 1;
                        int             currhitfloortexid = currhit.floor_tex_id - 1;
                        int             currhittileh = currhit.tile_height;
                        bool            currhitvertside = currhit.hit_vert_side;
                        bool            currhitbackface = currhit.back_face;


                        // correct hit result distance to rectify fish-eye effect
                        currhitdist = currhitdist * cosf(rayangle - player.yaw);

                        // projected hit result wall height in pixels
                        int             currhitwallh = (int)((currhittileh / currhitdist) * distproj);

                        // hit result wall top and bottom row
                        int             currhitwallbtmy = horizon + (int)(player.pos_z / (currhitdist / distproj));
                        int             currhitwalltopy = ((currhitwallbtmy - currhitwallh) < umost) ? umost : (currhitwallbtmy - currhitwallh);


                        // -- draw hit result walls -- //

                        if (!currhitbackface) {
                                texture_t       *walltex = &(texture_list.data[currhitwalltexid]);

                                if (!walltex->is_anim) {
                                        int             walltexoffsetx = currhitvertside ? (int)currhity % walltex->w : (int)currhitx % walltex->w;
                                        int             walldrawy = (currhitwallbtmy > drawnmosty) ? drawnmosty : currhitwallbtmy;

                                        if (currhitwalltopy < drawnmosty) {
                                                // hit result wall to be drawn is not completely occluded ...

                                                for (; (walldrawy >= currhitwalltopy) && (walldrawy > umost); --walldrawy) {
                                                        int     distwalltop = walldrawy + currhitwallh - horizon;
                                                        float   walltexscale = (float)walltex->h / (currhitwallh / ((float)currhittileh / walltex->h));
                                                        int     walltexoffsety = (int)(distwalltop * walltexscale) % walltex->h;

                                                        uint32_t walltexcolor = walltex->buffer[(walltex->w * walltexoffsety) + walltexoffsetx];
                                                        screenbuf[(g_screenbuf_width * walldrawy) + x] = walltexcolor;
                                                }

                                                // update top most drawn row
                                                drawnmosty = currhitwalltopy;
                                        }
                                }
                                else {
                                        int             animoffsetx = (walltex->frame_w * walltex->frame_index);
                                        int             walltexoffsetx = currhitvertside ? (int)currhity % walltex->frame_w : (int)currhitx % walltex->frame_w;
                                        int             walldrawy = (currhitwallbtmy > drawnmosty) ? drawnmosty : currhitwallbtmy;

                                        if (currhitwalltopy < drawnmosty) {
                                                // hit result wall to be drawn is not completely occluded ...

                                                for (; (walldrawy >= currhitwalltopy) && (walldrawy > umost); --walldrawy) {
                                                        int     distwalltop = walldrawy + currhitwallh - horizon;
                                                        float   walltexscale = (float)walltex->h / (currhitwallh / ((float)currhittileh / walltex->h));
                                                        int     walltexoffsety = (int)(distwalltop * walltexscale) % walltex->h;

                                                        uint32_t walltexcolor = walltex->buffer[(walltex->w * walltexoffsety) + (animoffsetx + walltexoffsetx)];
                                                        screenbuf[(g_screenbuf_width * walldrawy) + x] = walltexcolor;
                                                }

                                                // update top most drawn row
                                                drawnmosty = currhitwalltopy;
                                        }
                                }
                        }


                        // -- draw top side floors -- //

                        else {
                                if ((currhitwalltopy < drawnmosty) && (i > 0)) {
                                        ray_hit_t       prevhit = rayhits.data[i - 1];
                                        bool            prevhitbackface = prevhit.back_face;
                                        float           prevhitx = prevhit.hitpt_x;
                                        float           prevhity = prevhit.hitpt_y;

                                        // the current hit result must be the back face and the previous hit result must be the front face of the same tile
                                        if ((currhitbackface && !prevhitbackface) && (map_same_tile(map_id, currhitx, currhity, prevhitx, prevhity))) {
                                                if (currhitwalltopy < drawnmosty) {
                                                        // top side floor to be drawn is not completely occluded ...

                                                        for (int topsidedrawy = drawnmosty; (topsidedrawy >= currhitwalltopy) && (topsidedrawy > umost); --topsidedrawy) {
                                                                // distance from current draw row to horizon row
                                                                int     disthorizon = topsidedrawy - horizon;

                                                                // straight distance to floor intersection
                                                                float   distfloorstraight = ((player.pos_z - currhittileh) / (float)disthorizon) * distproj;

                                                                // actual distance to floor intersection
                                                                float   distflooractual = distfloorstraight / cosf(rayangle - player.yaw);

                                                                // floor intersection (world space) 
                                                                float   floorx = player.pos_x + cosf(rayangle) * distflooractual,
                                                                        floory = player.pos_y + sinf(rayangle) * distflooractual;

                                                                if ((floorx > 0) && (floory > 0) && (floorx < map->w) && (floory < map->h)) {
                                                                        texture_t       *floortex = &(texture_list.data[currhitfloortexid]);

                                                                        if (!floortex->is_anim) {
                                                                                int             floortexoffsetx = (int)floorx % floortex->w;
                                                                                int             floortexoffsety = (int)floory % floortex->h;

                                                                                uint32_t        floortexcolor = floortex->buffer[(floortex->w * floortexoffsety) + floortexoffsetx];
                                                                                screenbuf[(g_screenbuf_width * topsidedrawy) + x] = floortexcolor;
                                                                        }
                                                                        else {
                                                                                int             animoffsetx = (floortex->frame_w * floortex->frame_index);
                                                                                int             floortexoffsetx = (int)floorx % floortex->frame_w;
                                                                                int             floortexoffsety = (int)floory % floortex->h;

                                                                                uint32_t        floortexcolor = floortex->buffer[(floortex->w * floortexoffsety) + (animoffsetx + floortexoffsetx)];
                                                                                screenbuf[(g_screenbuf_width * topsidedrawy) + x] = floortexcolor;
                                                                        }
                                                                }
                                                        }
                                                }

                                                // update top most drawn row
                                                drawnmosty = currhitwalltopy;

                                                // update floor draw start row
                                                floorstarty = currhitwalltopy;
                                        }


                                }
                        }


                        // -- draw floors -- //

                        // determine bottom row of the wall as the draw end row for floor
                        floorendy = currhitwallbtmy;

                        if (floorendy < floorstarty) {
                                // floor to be drawn is not completely occluded ...

                                for (int floordrawy = floorstarty; floordrawy > floorendy; --floordrawy) {
                                        // distance from current draw row to horizon row
                                        int     disthorizon = floordrawy - horizon;

                                        // straight distance to floor intersection
                                        float   distfloorstraight = (player.pos_z / (float)disthorizon) * distproj;

                                        // actual distance to floor intersection
                                        float   distflooractual = distfloorstraight / cosf(rayangle - player.yaw);

                                        // floor intersection (world space)
                                        float   floorx = player.pos_x + cosf(rayangle) * distflooractual,
                                                floory = player.pos_y + sinf(rayangle) * distflooractual;

                                        if ((floorx > 0) && (floory > 0) && (floorx < map->w) && (floory < map->h)) {
                                                int             floortexid = map_get_floor_tex_id(map_id, floorx, floory) - 1;
                                                texture_t       *floortex = &(texture_list.data[floortexid]);

                                                if (!floortex->is_anim) {
                                                        int             floortexoffsetx = (int)floorx % floortex->w;
                                                        int             floortexoffsety = (int)floory % floortex->h;

                                                        uint32_t        floortexcolor = floortex->buffer[(floortex->w * floortexoffsety) + floortexoffsetx];
                                                        screenbuf[(g_screenbuf_width * floordrawy) + x] = floortexcolor;
                                                }
                                                else {
                                                        int             animoffsetx = (floortex->frame_w * floortex->frame_index);
                                                        int             floortexoffsetx = (int)floorx % floortex->frame_w;
                                                        int             floortexoffsety = (int)floory % floortex->h;

                                                        uint32_t        floortexcolor = floortex->buffer[(floortex->w * floortexoffsety) + (animoffsetx + floortexoffsetx)];
                                                        screenbuf[(g_screenbuf_width * floordrawy) + x] = floortexcolor;
                                                }
                                        }
                                }
                        }


                        // -- draw sky / ceiling -- //

                        if (toggle_sky && !str_empty(sky_tex_id)) {
                                texture_t       *skytex = texture_get_by_id(sky_tex_id);

                                if (skytex && (drawnmosty > umost)) {
                                        for (int skydrawy = drawnmosty; skydrawy > umost; --skydrawy) {
                                                int             skytexoffsetx = (int)floorf((rayangle * skytex->w) / (PI * 2.f)) % skytex->w;

                                                uint32_t        skytexcolor = skytex->buffer[(skytex->w * skydrawy) + skytexoffsetx];
                                                screenbuf[(g_screenbuf_width * skydrawy) + x] = skytexcolor;
                                        }
                                }
                                else if (!skytex && (drawnmosty > umost)) {
                                        for (int skydrawy = drawnmosty; skydrawy > umost; --skydrawy)
                                                screenbuf[(g_screenbuf_width * skydrawy) + x] = GET_OPAQUE_COLOR_HEX(0x80, 0x80, 0x80);
                                }
                        }
                        else {
                                for (int skydrawy = drawnmosty; skydrawy > umost; --skydrawy)
                                        screenbuf[(g_screenbuf_width * skydrawy) + x] = GET_OPAQUE_COLOR_HEX(0x80, 0x80, 0x80);
                        }



                        // update floor draw start row
                        floorstarty = drawnmosty;
                }
        }


        return true;
}

