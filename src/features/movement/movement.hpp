#pragma once
#include <globals.hpp>

class movement {
public:
    void on_create_move( c_user_cmd *cmd, const vector_3d &old_angles );
    void quick_stop( c_user_cmd *cmd );
    void fast_stop( c_user_cmd *cmd );
    void correct_movement( c_user_cmd *cmd, const vector_3d &wish_direction );
    void directional_strafe( c_user_cmd *cmd, const vector_3d &old_angles );

    void slow( c_user_cmd *cmd, float wish_speed );

    int ground_ticks = 0;
};

inline movement g_movement = { };