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

class prediction_context {
public:
    void post_think( c_base_entity *player );
    void start( c_user_cmd *ucmd );
    void finish( c_user_cmd *ucmd );
    void setup_move_data( c_cs_player *player, c_move_data *move_data );
    bool predict_player_entity( c_cs_player *player );
    void restore( );
    void simulate_tick( );

    prediction_data &get_prediction_info( const int current_command );

private:
    uintptr_t prediction_player;
    uintptr_t prediction_seed;
    c_cs_player *target = nullptr;

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

    vector_3d velocity = { };
    int predicted_flags = 0;
    bool backup_in_prediction = false;
    bool backup_first_time_predicted = false;
    c_move_data move_data = { };
}; 

inline prediction_context g_prediction_context = { };