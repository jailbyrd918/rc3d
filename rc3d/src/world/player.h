/*
        DESCRIPTION:
                World player definition and handling
*/

#ifndef PLAYER_H
#define PLAYER_H


struct {
        // -- translation -- /////////////////////////

        float   pos_x;
        float   pos_y;
        float   pos_z;

        int     move_dir;
        int     move_speed;
        float   curr_speed;
        bool    moving;
        int     height;


        // -- rotation -- ////////////////////////////

        float   yaw;
        int     yaw_dir;
        int     yaw_speed;
        float   fov;
}
player;


// >> initializes player properties
// >>> [arg] turn_speed: enter the value in degree, not radian
// >>> [arg] field_of_view: enter the value in degree, not radian
void player_init
(const float start_x, const float start_y, const char *map_id, const float player_height, const int move_speed, const int turn_speed, const int field_of_view);

// >> moves player, updates player logic, etc.
bool player_update
(const char *map_id, const float delta_time);

bool player_render
(const char *map_id);


#endif
