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
    bool handle_auto_direction( c_user_cmd *cmd );
    void handle_direction( c_user_cmd *cmd );
    void handle_fake( c_user_cmd *cmd );
    void handle_real( c_user_cmd *cmd );
    int get_max_choke_ticks( );
    void fake_walk( c_user_cmd *cmd );
    void handle_pitch( c_user_cmd *cmd );
    void distortion( c_user_cmd *cmd );
    void on_create_move( c_user_cmd *cmd, vector_3d vangle );

    struct freestand_record_t {
        void reset( ) {
            this->left_damage = this->right_damage = 0.0f;
            this->left_fraction = this->right_fraction = 0.0f;
        }

        freestand_record_t( ) {
            reset( );
        }

        float left_damage, right_damage;
        float left_fraction, right_fraction;
    } direction_info;

    float dir;
    manual_direction manual_dir;
    bool using_manual_dir;
    bool switch_distortion_side;
    float distortion_timer;
    vector_3d real;
};

inline antiaim g_antiaim = { };