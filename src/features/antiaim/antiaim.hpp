#pragma once
#include <globals.hpp>

enum class manual_direction : int {
    none = -1,
    left,
    right,
    back
};

class antiaim {
public:
    void update_manual_direction( );
    void handle_direction( c_user_cmd *cmd );
    void handle_fake( c_user_cmd *cmd );
    void handle_real( c_user_cmd *cmd );
    int get_max_choke_ticks( );
    void fake_walk( c_user_cmd *cmd );
    void handle_pitch( c_user_cmd *cmd );
    void distortion( c_user_cmd *cmd );
    void on_create_move( c_user_cmd *cmd, vector_3d vangle );

    float dir;
    manual_direction manual_dir;
    bool using_manual_dir;
    bool switch_distortion_side;
    float distortion_timer;
    vector_3d real;
};

inline antiaim g_antiaim = { };