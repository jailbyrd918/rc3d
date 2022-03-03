#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>

#include "graphics/display.h"
#include "world/map.h"
#include "world/player.h"

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
}

static void _engine_render
(void)
{
        display_clear_screenbuf(GET_OPAQUE_COLOR_HEX(0x3a, 0x3b, 0x3c));

        switch (display_mode) {
                case DISPLAY_MODE_2D:
                        map_render(curr_map_id);
                        player_render(curr_map_id);
                        break;

                case DISPLAY_MODE_3D:
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
        engineok &= (display_init("Raycast", 1024, 768, 420, 340));

        // check if engine is ok to run
        engine_loop = (engineok) ? true : false;


        // -- allocate resources and initialize engine components -- //

        map_init_list();
        map_add_to_list("demo", "./resrc/maps/demo.txt");
        curr_map_id = "demo";

        display_mode = DISPLAY_MODE_3D;

        player_init(400, 600, 5, 120, 60);

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
        display_free();

        // unload IMG_PNG libraries and cleans up initialized SDL core subsystems 
        IMG_Quit();
        SDL_Quit();
}

