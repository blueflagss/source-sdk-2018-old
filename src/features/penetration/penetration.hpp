#pragma once
#include <globals.hpp>

class c_fire_bullet_data {
public:
    int penetrate_count;
    float trace_length;
    float remaining_length;
    float damage;
    float in_damage;
    float out_damage;
    int out_hitgroup;
};

class penetration_system {
public:
    c_fire_bullet_data run( const vector_3d src, const vector_3d end, c_cs_player *ent, bool is_zeus = false, c_cs_weapon_info *info_override = nullptr );
    bool simulate_fire_bullet( const c_cs_weapon_info *data, vector_3d src, vector_3d pos, c_fire_bullet_data &fire_info, bool is_zeus = false, c_cs_player *ent = nullptr );
    bool is_breakable_entity( c_cs_player *ent );
    bool trace_to_exit( vector_3d start, vector_3d dir, vector_3d &end, c_game_trace &trace_enter, c_game_trace &trace_exit, float step_size, float max_distance );
    bool handle_bullet_penetration( c_cs_player *ent, float &penetration, int &enter_material, bool &hit_grate, c_game_trace &tr, vector_3d &direction, surfacedata_t *surface_data, float penetration_modifier, float damage_modifier, float penetration_power, int &penetration_count, vector_3d &src, float distance, float current_distance, float &current_damage, c_game_trace &exit_trace );
    float scale_damage( c_cs_player *player, float damage, float armor_ratio, int hitgroup, bool is_zeus = false );
    void clip_trace_to_player( c_cs_player *player, const vector_3d &vec_abs_start, const vector_3d &vec_abs_end, unsigned int mask, c_trace_filter *filter, c_game_trace *tr );
    void clip_trace_to_players( const vector_3d &start, const vector_3d &end, unsigned int mask, c_trace_filter_hitscan *filter, c_game_trace *tr );
};

inline penetration_system g_penetration{ };