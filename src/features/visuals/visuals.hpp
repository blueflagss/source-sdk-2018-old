#pragma once
#include <globals.hpp>
#include <features/animations/animation_sync.hpp>

class visuals {
public:
    void render( );
    void render_indicators( );
    void render_hitmarker( );
    void render_hud_scope( );
    void world_modulation( );

    float hitmarker_fraction = 0.0f;

private:
    int distance_offset = 0;
    std::deque< float > opacity_array;

private:
    void render_player( c_cs_player *player );
    void render_offscreen( c_cs_player *player ) const;
    void render_skeleton( c_cs_player *player, lag_record *record, color skeleton_color ) const;
};

inline visuals g_visuals = { };