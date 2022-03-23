#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>

#include "utils/maths.h"
#include "graphics/display.h"
#include "graphics/texture.h"
#include "world/map.h"
#include "world/player.h"
#include "world/ray.h"
#include "world/sprite.h"
#include "graphics/proj.h"

#include "engine.h"


#define FRAMES_PER_SEC          (120)
#define TARGET_FRAME_TICK       (1000 / FRAMES_PER_SEC)

static  int     old_ticked = 0;
static  float   delta_time = 0.f;

// >> engine loop status
static  bool    engine_loop = false;

// >> resources (textures, maps, etc.) directory
char            *resrc_dir = "";

// >> current map id
char            *curr_map_id = "";


static void _update_delta_time
(void)
{
        // delay duration until the target frame tick is reached
        int delaytime = TARGET_FRAME_TICK - (SDL_GetTicks() - old_ticked);
        if (delaytime > 0 && delaytime <= TARGET_FRAME_TICK)
                SDL_Delay(delaytime);

        // calculate and update delta time
        delta_time = (SDL_GetTicks() - old_ticked) / 1000.f;

        // update old ticked frame
        old_ticked = SDL_GetTicks();
}



static void _engine_proc_input
(void)
{
        SDL_Event event;
        SDL_PollEvent(&event);

        switch (event.type) {

                case SDL_QUIT:
                        engine_loop = false;
                        break;


                case SDL_KEYDOWN:
                        switch (event.key.keysym.sym) {
                                case SDLK_ESCAPE:
                                        engine_loop = false;
                                        break;

                                case SDLK_m:
                                        display_toggle_mode();
                                        break;

                                case SDLK_UP:
                                        // perform skidding
                                        if (player.move_dir == -1)
                                                player.curr_speed = 0;

                                        player.move_dir = 1;
                                        player.moving = true;
                                        break;

                                case SDLK_DOWN:
                                        // perform skidding
                                        if (player.move_dir == 1)
                                                player.curr_speed = 0;

                                        player.move_dir = -1;
                                        player.moving = true;
                                        break;

                                case SDLK_LEFT:
                                        player.yaw_dir = -1;
                                        break;

                                case SDLK_RIGHT:
                                        player.yaw_dir = 1;
                                        break;

                                        // used for debug breakpoint at any frame
                                case SDLK_d: { const char *dbg = "debug"; } break;

                                default:
                                        break;
                        }
                        break;


                case SDL_KEYUP:
                        switch (event.key.keysym.sym) {
                                case SDLK_UP:
                                case SDLK_DOWN:
                                        player.moving = false;
                                        break;

                                case SDLK_LEFT:
                                case SDLK_RIGHT:
                                        player.yaw_dir = 0;
                                        break;

                                default:
                                        break;
                        }
                        break;


                default:
                        break;

        }
}

static void _engine_update
(void)
{
        _update_delta_time();

        player_update(curr_map_id, delta_time);
        ray_cast_all(curr_map_id);
        texture_update_all_anims(delta_time);
        sprite_check_visible();

}

static void _engine_render
(void)
{
        display_clear_screenbuf(GET_OPAQUE_COLOR_HEX(0x3a, 0x3b, 0x3c));

        switch (display_mode) {
                case DISPLAY_MODE_2D:
                        map_draw_2d(curr_map_id);
                        ray_draw_all(curr_map_id);
                        player_draw_2d(curr_map_id);
                        sprite_draw_2d(curr_map_id);
                        break;

                case DISPLAY_MODE_3D:
                        projection_3d_implement(curr_map_id, true, "sky_cloudy");
                        break;

                default:
                        break;
        }

        display_render_screenbuf();
        SDL_RenderPresent(renderer);
}


void engine_init
(void)
{
        // >> true: engine is good to run
        // >> false: engine failed/bad to run -> would crash if proceeds
        bool engineok = true;
        engineok &= (SDL_Init(SDL_INIT_EVERYTHING) == 0);               // initialize SDL core subsystems
        engineok &= (IMG_Init(IMG_INIT_PNG) != 0);                      // load SDL IMG_PNG libraries
        engineok &= (display_init("rc3d_demo", 1024, 768, 420, 340));

        // check if engine is ok to run
        engine_loop = (engineok) ? true : false;


        // -- allocate resources and initialize engine components -- //

        map_init_list();
        map_add_to_list("demo", "./resrc/maps/rc3dm_demo.txt");
        map_add_to_list("park", "./resrc/maps/rc3dm_park.txt");
        curr_map_id = "park";

        display_mode = DISPLAY_MODE_3D;

        player_init(30 * TILE_SIZE, 30 * TILE_SIZE, curr_map_id, 45, 20, 120, 60);

        ray_init_list();

        texture_init_list();
        texture_add_to_list("wall_brick", "./resrc/textures/brick_wall.png", 64, 1, 0.f);
        texture_add_to_list("wall_brick_2", "./resrc/textures/brick_wall_2.png", 64, 1, 0.f);
        texture_add_to_list("wall_hedge", "./resrc/textures/hedge.png", 64, 1, 0.f);
        texture_add_to_list("floor_grass", "./resrc/textures/grass.png", 64, 1, 0.f);
        texture_add_to_list("floor_water", "./resrc/textures/anim_water.png", 64, 32, .2f);
        texture_add_to_list("red_brick", "./resrc/textures/red_brick.png", 64, 1, 0.f);
        texture_add_to_list("wall_waterfall", "./resrc/textures/anim_waterfall.png", 64, 16, .1f);
        texture_add_to_list("wall_pattern", "./resrc/textures/vinelike_pattern.png", 64, 1, 0.f);
        texture_add_to_list("floor_concrete", "./resrc/textures/concrete.png", 64, 1, 0.f);
        texture_add_to_list("floor_rocky", "./resrc/textures/rocky_road.png", 64, 1, 0.f);
        texture_add_to_list("floor_wood_v", "./resrc/textures/wooden_floor_v.png", 64, 1, 0.f);
        texture_add_to_list("floor_wood_h", "./resrc/textures/wooden_floor_h.png", 64, 1, 0.f);
        texture_add_to_list("sky_cloudy", "./resrc/textures/cloudy_sky.png", 64, 1, 0.f);
        texture_add_to_list("sky_nightcity", "./resrc/textures/night_city_sky.png", 64, 1, 0.f);
        texture_add_to_list("spr_tree_1", "./resrc/textures/anim_tree_1.png", 150, 12, 0.2f);
        texture_add_to_list("spr_tree_2", "./resrc/textures/anim_tree_2.png", 200, 20, 0.2f);
        texture_add_to_list("spr_pine", "./resrc/textures/pine_tree.png", 64, 1, 0.f);
        texture_add_to_list("spr_rgb", "./resrc/textures/anim_rgb.png", 64, 3, .5f);

        sprite_init_lists();
        sprite_add_to_list("pine1", curr_map_id, 512, 1024, "spr_pine");
        sprite_add_to_list("pine2", curr_map_id, 512, 1536, "spr_pine");
        sprite_add_to_list("pine3", curr_map_id, 1920, 1024, "spr_pine");
        sprite_add_to_list("pine4", curr_map_id, 960, 576, "spr_pine");
        sprite_add_to_list("pine5", curr_map_id, 1472, 576, "spr_pine");
        sprite_add_to_list("pine6", curr_map_id, 1920, 1536, "spr_pine");
        sprite_add_to_list("pine7", curr_map_id, 1472, 1984, "spr_pine");
        sprite_add_to_list("pine8", curr_map_id, 960, 1984, "spr_pine");
        sprite_add_to_list("tree1_1", curr_map_id, 512, 1728, "spr_tree_1");
        sprite_add_to_list("tree1_2", curr_map_id, 512, 832, "spr_tree_1");
        sprite_add_to_list("tree1_3", curr_map_id, 768, 576, "spr_tree_1");
        sprite_add_to_list("tree1_4", curr_map_id, 1664, 576, "spr_tree_1");
        sprite_add_to_list("tree1_5", curr_map_id, 1920, 832, "spr_tree_1");
        sprite_add_to_list("tree1_6", curr_map_id, 1920, 1792, "spr_tree_1");
        sprite_add_to_list("tree1_7", curr_map_id, 1664, 1984, "spr_tree_1");
        sprite_add_to_list("tree1_8", curr_map_id, 768, 1984, "spr_tree_1");
        sprite_add_to_list("tree2_1", curr_map_id, 512, 1984, "spr_tree_2");
        sprite_add_to_list("tree2_2", curr_map_id, 512, 576, "spr_tree_2");
        sprite_add_to_list("tree2_3", curr_map_id, 1920, 1984, "spr_tree_2");
        sprite_add_to_list("tree2_4", curr_map_id, 1920, 576, "spr_tree_2");

}

void engine_run
(void)
{
        do {
                _engine_proc_input();
                _engine_update();
                _engine_render();

        } while (engine_loop);
}

void engine_quit
(void)
{
        // destroy engine components and deallocate resources
        map_free_list();
        ray_free_list();
        texture_free_list();
        sprite_free_lists();
        display_free();

        // unload IMG_PNG libraries and cleans up initialized SDL core subsystems 
        IMG_Quit();
        SDL_Quit();
}

