#pragma once
#include <globals.hpp>
#include <features/animations/animation_sync.hpp>

class visuals {
public:
    void render( );
    void render_hitmarker( );
    void world_modulation( );
    static void update_view_matrix( );

    float hitmarker_fraction = 0.0f;

private:
    int distance_offset = 0;
    std::deque< float > opacity_array;

private:
    void animate( c_cs_player *player );
    void render_player( c_cs_player *player );
    void draw_hud_scope( );
    void render_offscreen( c_cs_player *player, const player_info_t &player_info ) const;
    void render_skeleton( c_cs_player *player, lag_record *record, color skeleton_color ) const;
    //void render_skeleton( c_cs_player *player, lag_record *record, color bones_color ) const;
};

inline visuals g_visuals = { };