#pragma once
#include <features/aimbot/aimbot.hpp>

class resolver {
public:
    void update( lag_record *previous, lag_record *record );
    lag_record *find_ideal_record( aim_player *data );
    lag_record *find_last_record( aim_player *data );
};

inline resolver g_resolver;