#pragma once
#include <features/ragebot/ragebot.hpp>

class resolver {
public:
    void on_proxy_update( c_cs_player *player, float updated_value );
    bool anti_freestanding( lag_record &record );
    int Freestand( c_cs_player *player, lag_record *record, float a3, float a4, int *a14, int *a15 );
    double legacy( c_cs_player *player, lag_record *record );
    double legacy( );
    void start( c_cs_player *player, lag_record &record, lag_record *previous );
    lag_record *find_ideal_record( c_cs_player *player );
    lag_record *find_last_record( c_cs_player *player );
};

inline resolver g_resolver;