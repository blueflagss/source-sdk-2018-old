#pragma once
#include <globals.hpp>

class antiaim {
public:
    void handle_direction( c_user_cmd *cmd );
    void handle_fake( c_user_cmd *cmd );
    void handle_real( c_user_cmd *cmd );
    void handle_pitch( c_user_cmd *cmd );
    void on_create_move( c_user_cmd *cmd, vector_3d vangle );

    float dir;
    vector_3d real;
};

inline antiaim g_antiaim = { };