#pragma once
#include <utils/utils.hpp>

class c_view_vectors {
public:
    __forceinline c_view_vectors( ) {}

    __forceinline c_view_vectors( vector_3d view, vector_3d hull_min, vector_3d hull_max, vector_3d duck_hull_min, vector_3d duck_hull_max, vector_3d duck_view, vector_3d obs_hull_min, vector_3d obs_hull_max, vector_3d dead_view_height ) {
        view = view;
        hull_min = hull_min;
        hull_max = hull_max;
        duck_hull_min = duck_hull_min;
        duck_hull_max = duck_hull_max;
        duck_view = duck_view;
        obs_hull_min = obs_hull_min;
        obs_hull_max = obs_hull_max;
        dead_view_height = dead_view_height;
    }

    vector_3d view;
    vector_3d hull_min;
    vector_3d hull_max;
    vector_3d duck_hull_min;
    vector_3d duck_hull_max;
    vector_3d duck_view;
    vector_3d obs_hull_min;
    vector_3d obs_hull_max;
    vector_3d dead_view_height;
};

class c_game_rules {
public:
    c_view_vectors *get_view_vectors( ) {
        return utils::get_method< c_view_vectors *( __thiscall * ) ( void * ) >( this, 31 )( this );
    }
};