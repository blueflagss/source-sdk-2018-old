#pragma once
#include <globals.hpp>

class sound_player {
public:
    sound_player( ) {
        reset( );
    }

    __forceinline void reset( ) {
        this->origin = vector_3d( );
        this->player = nullptr;
        this->last_update_time = -1;
        this->visible = false;
    }

    vector_3d origin;
    c_cs_player *player;
    int last_update_time;
    bool visible;
};

class sound_handler {
public:
    void on_create_move( );
    void reset_data( );
    void on_round_start( event_t *evt );
    void update_position( c_cs_player *player );
    void set_dormant_origin( sndinfo_t *sound );

    std::array< sound_player, 64 > dormant_players;
};

inline sound_handler g_sound_handler{ };