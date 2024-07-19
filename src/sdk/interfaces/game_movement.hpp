#pragma once
#include <utils/utils.hpp>

class c_move_data {
public:
    bool first_run_of_functions : 1;
    bool game_code_moved_player : 1;
    bool no_air_control : 1;
    unsigned long player_handle;
    int impulse_command;
    vector_3d view_angles;
    vector_3d abs_view_angles;
    int buttons;
    int old_buttons;
    float forward_move;
    float side_move;
    float upmove;
    float max_speed;
    float client_max_speed;
    vector_3d velocity;
    vector_3d trailing_velocity;
    float trailing_vel_time;
    vector_3d ang;
    vector_3d old_ang;
    float step_height;
    vector_3d wish_vel;
    vector_3d jump_vel;
    vector_3d constraint_center;
    float constraint_radius;
    float constraint_width;
    float constraint_speed_factor;
    bool constraint_past_radius;
    vector_3d abs_origin;
};

class c_base_player;

class c_game_movement {
public:
    virtual ~c_game_movement( void ) = 0;
    virtual void process_movement( void *player, c_move_data *move ) = 0;
    virtual void reset( void ) = 0;
    virtual void start_track_prediction_errors( void *player ) = 0;
    virtual void finish_track_prediction_errors( void *player ) = 0;
    virtual void diff_print( char const *fmt, ... ) = 0;
    virtual vector_3d const &get_player_mins( bool ducked ) const = 0;
    virtual vector_3d const &get_player_maxs( bool ducked ) const = 0;
    virtual vector_3d const &get_player_view_offset( bool ducked ) const = 0;
    virtual bool is_moving_player_stuck( void ) const = 0;
    virtual void *get_moving_player( void ) const = 0;
    virtual void unblock_pusher( void *player, void *pusher ) = 0;
    virtual void setup_movement_bounds( void *move ) = 0;
};