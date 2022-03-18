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
        float   distproj = (g_screenbuf_width >> 1) / tanf(player.fov / 2);


        // -- iterate ray by ray -- //

        for (int x = 0; x < g_screenbuf_width; ++x) {
                float           rayangle = ray_list[x].angle;
                ray_hit_list_t  rayhits = ray_list[x].hit_results;


                // -- draw tracking properties -- //

                int     drawnmosty = dmost;
                int     floorstarty = dmost;
                int     floorendy = dmost;


                for (size_t i = 0; i < rayhits.size; ++i) {

                        // -- current ray hit result properties -- //

                        ray_hit_t       currhit = rayhits.data[i];

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
                                                        int     distscrbuftop = walldrawy + currhitwallh - horizon;
                                                        float   walltexscale = (float)walltex->h / (currhitwallh / ((float)currhittileh / walltex->h));
                                                        int     walltexoffsety = (int)(distscrbuftop * walltexscale) % walltex->h;

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
                                                        int     distscrbuftop = walldrawy + currhitwallh - horizon;
                                                        float   walltexscale = (float)walltex->h / (currhitwallh / ((float)currhittileh / walltex->h));
                                                        int     walltexoffsety = (int)(distscrbuftop * walltexscale) % walltex->h;

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
                                if (currhitwalltopy < drawnmosty) {
                                        ray_hit_t       prevhit = rayhits.data[i - 1];
                                        bool            prevhitbackface = prevhit.back_face;
                                        float           prevhitx = prevhit.hitpt_x;
                                        float           prevhity = prevhit.hitpt_y;

                                        // the current hit result must be the back face and the previous hit result must be the front face of the same tile
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

                                        // update top most drawn row and floor draw start row
                                        floorstarty = drawnmosty = currhitwalltopy;
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


                        // update floor draw start row
                        floorstarty = drawnmosty;
                }


                // -- draw sky / ceiling -- //

                if (toggle_sky && !str_empty(sky_tex_id)) {
                        texture_t       *skytex = texture_get_by_id(sky_tex_id);

                        if (skytex && (drawnmosty > umost)) {
                                for (int skydrawy = drawnmosty; skydrawy > umost; --skydrawy) {
                                        int             skytexoffsetx = (int)floorf((rayangle * skytex->w) / (PI * 2.f)) % skytex->w;
                                        int             skytexoffsety = skydrawy % skytex->h;

                                        uint32_t        skytexcolor = skytex->buffer[(skytex->w * skytexoffsety) + skytexoffsetx];
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
        }


        // -- draw sprites -- //

        for (size_t i = 0; i < visible_sprite_list.size; ++i) {

                // -- current visible sprite properties -- //

                sprite_t        *currsprite = visible_sprite_list.sprite_refs[i];

                float           currspritedist = currsprite->dist;
                float           currspriteangle = currsprite->angle;


                // rendering sprite angle (facing player)
                float           spriteangle = atan2f(currsprite->pos_y - player.pos_y, currsprite->pos_x - player.pos_x) - player.yaw;

                // correct sprite distance to rectify fish-eye effect
                currspritedist = currspritedist * cosf(currspriteangle);

                // current visible sprite texture 
                texture_t       *spritetex = texture_get_by_id(currsprite->tex_id);

                // texture width and height
                int             texw = (spritetex->is_anim) ? spritetex->frame_w : spritetex->w;
                int             texh = spritetex->h;

                // projected sprite width and height in pixels
                float           spritew = ((float)texw / currspritedist) * distproj;
                float           spriteh = ((float)texh / currspritedist) * distproj;

                // sprite left-most and right-most column
                float           spritex = tanf(spriteangle) * distproj;
                int             spritelmost = (int)(spritex + (g_screenbuf_width >> 1) - (spritew / 2));
                int             spritermost = (int)(spritelmost + spritew);

                // sprite top and bottom row
                int             spritebtmy = (int)(horizon + ((player.pos_z - currsprite->pos_z) / (currspritedist / distproj)));
                int             spritetopy = ((int)(spritebtmy - spriteh) < umost) ? umost : (int)(spritebtmy - spriteh);
                                
                for (int spritedrawx = spritelmost; spritedrawx < spritermost; ++spritedrawx) {
                        if (spritedrawx >= 0 && spritedrawx < g_screenbuf_width) {

                                float           rayangle = ray_list[spritedrawx].angle;
                                ray_hit_list_t  rayhits = ray_list[spritedrawx].hit_results;

                                
                                // -- check for possible occlusion -- //

                                // sprite draw limit bottom row
                                int             spritedrawlimity = spritebtmy;

                                // index for closest wall in front of the sprite
                                int             distrayhitidx = -1;
                                for (int idx = 0; ((size_t)idx < rayhits.size) && (rayhits.data[idx].dist < currspritedist); distrayhitidx = idx++);

                                // determine sprite start drawing row (remove any occluded part)
                                for (int idx = 0; idx <= distrayhitidx; ++idx) {
                                        ray_hit_t       rayhit = rayhits.data[idx];
                                        float           rayhitdist = rayhit.dist * cosf(rayangle - player.yaw);
                                        int             rayhitwallh = (int)((rayhit.tile_height / rayhitdist) * distproj);
                                        int             rayhitwallbtmy = horizon + (int)(player.pos_z / (rayhitdist / distproj));
                                        int             rayhitwalltopy = ((rayhitwallbtmy - rayhitwallh) < umost) ? umost : (rayhitwallbtmy - rayhitwallh);

                                        spritedrawlimity = (rayhitwalltopy <= spritedrawlimity) ? rayhitwalltopy : spritedrawlimity;
                                }


                                if (!spritetex->is_anim) {
                                        int spritetexoffsetx = (int)((spritedrawx - spritelmost) * (texw / spritew)) % texw;

                                        for (int spritedrawy = spritetopy; spritedrawy <= spritedrawlimity; ++spritedrawy) {
                                                if (spritedrawy >= umost && spritedrawy < dmost) {
                                                        int             distscrbuftop = (int)((spritedrawy + spriteh) - ((player.pos_z - currsprite->pos_z) / (currspritedist / distproj)) - horizon);
                                                        int             spritetexoffsety = (int)(distscrbuftop * (texh / spriteh));
                                                        spritetexoffsety = CLAMP(spritetexoffsety, 0, texh - 1);

                                                        uint32_t        spritetexcolor = spritetex->buffer[(spritetex->w * spritetexoffsety) + spritetexoffsetx];

                                                        // draw ignoring transparent pixel
                                                        if (spritetexcolor != 0x00)
                                                                screenbuf[(g_screenbuf_width * spritedrawy) + spritedrawx] = spritetexcolor;
                                                }
                                        }
                                }
                                else {
                                        int     animoffsetx = (spritetex->frame_w * spritetex->frame_index);
                                        int     spritetexoffsetx = (int)((spritedrawx) * (texw / spritew)) % texw;

                                        for (int spritedrawy = spritetopy; spritedrawy <= spritedrawlimity; ++spritedrawy) {
                                                if (spritedrawy >= umost && spritedrawy < dmost) {
                                                        int             distscrbuftop = (int)((spritedrawy + spriteh) - ((player.pos_z - currsprite->pos_z) / (currspritedist / distproj)) - horizon);
                                                        int             spritetexoffsety = (int)(distscrbuftop * (texh / spriteh));
                                                        spritetexoffsety = CLAMP(spritetexoffsety, 0, texh - 1);

                                                        uint32_t        spritetexcolor = spritetex->buffer[(spritetex->w * spritetexoffsety) + (animoffsetx + spritetexoffsetx)];

                                                        // draw ignoring transparent pixel
                                                        if (spritetexcolor != 0x00)
                                                                screenbuf[(g_screenbuf_width * spritedrawy) + spritedrawx] = spritetexcolor;
                                                }
                                        }
                                }
                        }
                }
        }

        return true;
}

