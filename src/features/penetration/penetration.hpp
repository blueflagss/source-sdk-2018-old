#pragma once
#include <globals.hpp>

class c_fire_bullet_data {
public:
    int penetrate_count{ };
    int impact_count{ };
    float damage{ };
    float in_damage{ };
    float out_damage{ };
    int out_hitgroup{ };
    std::array< vector_3d, 6 > impacts{ };
    bool did_hit{ };
    vector_3d bullet_end;
};

class penetration_system {
public:
    inline static constexpr auto ray_extension = 40.f;

    c_fire_bullet_data run( const vector_3d src, const vector_3d end, c_cs_player *ent, const float &in_damage, const std::array< matrix_3x4, 128 > &bones, bool is_zeus = false, c_cs_weapon_info *info_override = nullptr );
    bool simulate_fire_bullet( const c_cs_weapon_info *data, vector_3d src, vector_3d pos, c_fire_bullet_data &fire_info, const std::array< matrix_3x4, 128 > &bones, bool is_zeus = false, c_cs_player *ent = nullptr );
    bool is_breakable_entity( c_base_entity *ent );
    bool trace_to_exit( c_game_trace &enter_trace, c_game_trace &exit_trace, vector_3d start_pos, vector_3d direction );
    bool handle_bullet_penetration( const c_cs_weapon_info *weapon_data, c_game_trace &enter_trace, vector_3d &eye_position, vector_3d diretion, int &penetration_count, float &current_damage, float penetration_power );
    float scale_damage( c_cs_player *player, float damage, float armor_ratio, int hitgroup, bool is_zeus = false );
    void clip_trace_to_player( c_cs_player *player, const vector_3d &vec_abs_start, const vector_3d &vec_abs_end, unsigned int mask, c_trace_filter *filter, c_game_trace *tr, float max_distance = 60.0f );
    void clip_trace_to_players( const vector_3d &start, const vector_3d &end, unsigned int mask, c_trace_filter_hitscan *filter, c_game_trace *tr );
    bool trace_ray( const vector_3d &min, const vector_3d &max, const matrix_3x4 &mat, float r, const vector_3d &src, const vector_3d &dst );
    bool proxy_trace_to_studio_csgo_hitgroups_priority( c_cs_player *ent, uint32_t contents_mask, vector_3d *origin, c_game_trace *tr, ray_t *ray, matrix_3x4 **mat );
};

inline penetration_system g_penetration{ };