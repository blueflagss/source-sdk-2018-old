#pragma once
#include <globals.hpp>

class tickbase {
public:
    void on_create_move( );
    void adjust_limit( bool finalize );
    int current_limit( int cmd_num );

    // vars
    int wish_shift;
    int recharge;
    int limit;
    int correction;

    bool choke;
    bool enabled;
    bool post;
};

inline tickbase g_tickbase = { };