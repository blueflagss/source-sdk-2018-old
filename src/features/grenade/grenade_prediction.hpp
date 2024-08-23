#pragma once
#include <globals.hpp>

class grenade_trajectory {
public:
    grenade_trajectory( ) = default;

    grenade_trajectory( c_base_entity *entity, bool show_bounces ) {
        this->entity = entity;
        this->show_bounces = show_bounces;
    }

    std::vector< std::pair< vector_3d, bool > > bounces;
    std::vector< std::tuple< vector_3d, bool, vector_3d > > path;
        
    vector_3d origin;
    int last_update_tick;
    bool show_bounces;

    int physics_clip_velocity( const vector_3d &in, const vector_3d &normal, vector_3d &out, float overbounce );
    void push_entity( c_game_trace &trace, c_base_entity *player, const vector_3d &src, vector_3d &angle );
    void resolve_fly_collision_custom( c_game_trace &trace, vector_3d &vecVelocity, float interval );
    void on_grenade_thrown( event_t *evt );
    void render_path( );
    void simulate_path( );
    void physics_add_gravity_move( c_base_entity *player, vector_3d &move );
    bool check_detonate( c_cs_weapon_base *weapon, const c_game_trace &tr, int tick, float interval );
    int tick( c_base_entity *ent, c_cs_weapon_base *weapon, vector_3d &src, int tick, float interval );

    c_base_entity *entity;
    vector_3d vel;
    int log_timer;
    bool did_collide, detonated, fire;
    std::vector< std::tuple< vector_3d, bool, vector_3d > > current_path, saved_path;
    int last_weapon_id;
    float spawn_time;
    float detonate_time;
};

inline grenade_trajectory g_grenade_prediction = { };