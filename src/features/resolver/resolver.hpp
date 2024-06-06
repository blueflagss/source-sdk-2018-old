#pragma once
#include <features/aimbot/aimbot.hpp>

class resolver {
public:
    lag_record *find_ideal_record( aim_player *data );
    lag_record *find_last_record( aim_player *data );
};

inline resolver g_resolver;