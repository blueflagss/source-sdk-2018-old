#pragma once
#include <features/ragebot/ragebot.hpp>

class adaptive_angle {
public:
    float yaw;
    float dist;

public:
    adaptive_angle( float yaw, float penalty = 0.f ) {
        yaw = math::normalize_angle( yaw );
        dist = 0.f;
        dist -= penalty;
    }
};

class resolver {
public:
    void on_proxy_update( c_cs_player *player, float updated_value );
    bool anti_freestanding( lag_record &record );
    std::pair< float, bool > AntiFreestand( c_cs_player *player, lag_record *record, bool include_base, float base_yaw, float delta );
    int Freestand( c_cs_player *player, lag_record *record, float a3, float a4, int *a14, int *a15 );
    double legacy( c_cs_player *player, lag_record *record );
    double legacy( );
    void start( c_cs_player *player, lag_record &record, lag_record *previous );
    lag_record *find_ideal_record( c_cs_player *player );
    lag_record *find_last_record( c_cs_player *player );
};

inline resolver g_resolver;