#pragma once
#include <globals.hpp>

class movement {
public:
    void on_create_move( c_user_cmd *cmd, const vector_3d &old_angles );
    void correct_movement( c_user_cmd *cmd, const vector_3d &wish_direction );
    void directional_strafe( c_user_cmd *cmd, const vector_3d &old_angles );
};

inline movement g_movement = { };