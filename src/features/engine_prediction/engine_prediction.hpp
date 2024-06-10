#pragma once
#include <globals.hpp>

struct prediction_data {
    int command_sequence;

    void reset( ) {
        this->command_sequence = 0;
    }

    struct tickbase_t {
        int limit = { };
        int sent_commands = { };
        int choked_commands = { };
    } tickbase = { };
};

class engine_prediction {
public:
    void post_think( c_base_entity *player );
    void start( c_user_cmd *ucmd );
    void finish( c_user_cmd *ucmd );

    prediction_data &get_prediction_info( const int current_command );

private:
    uintptr_t prediction_player;
    uintptr_t prediction_seed;

public:
    int backup_tick_base = 0.f;
    float backup_curtime = 0.f;
    float backup_frametime = 0.f;
    float weapon_cycle;
    float weapon_sequence;
    float weapon_animtime;
    int sequence = 0;
    int animation_parity = 0;
    float ideal_inaccuracy = 0.f;

    std::array< prediction_data, 90 > pred_data;

    vector_3d predicted_velocity = { };
    int predicted_flags = 0;
    bool backup_in_prediction = false;
    bool backup_first_time_predicted = false;
    c_move_data move_data = { };
}; 

inline engine_prediction g_prediction = { };