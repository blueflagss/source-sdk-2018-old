#pragma once
#include <globals.hpp>

class grenade_warning {
public:
    grenade_warning( ) = default;

    grenade_warning( c_base_entity *entity, vector_3d velocity, int weapon_id ) {
        this->entity = entity;
        this->weapon_id = weapon_id;
        this->vel = velocity;
        this->next_think_tick = 0;
        this->spawn_time = 0;
        this->detonated = false;
    }

    std::vector< std::tuple< vector_3d, bool > > path;

    vector_3d origin;
    int last_update_tick;
    int next_think_tick;

    void set_next_think_tick( float thinkTime );
    int tick( c_base_entity *ent, vector_3d &src, int tick, float interval );
    int physics_clip_velocity( const vector_3d &in, const vector_3d &normal, vector_3d &out, float overbounce );
    void push_entity( c_game_trace &trace, c_base_entity *player, const vector_3d &src, vector_3d &angle );
    void resolve_fly_collision_custom( c_game_trace &trace, vector_3d &vecVelocity, float interval );
    void on_grenade_thrown( event_t *evt );
    void render_path( );
    void simulate_path( );
    void physics_add_gravity_move( c_base_entity *player, vector_3d &move );
    bool check_detonate( c_cs_weapon_base *weapon, const c_game_trace &tr, int tick, float interval );

    c_base_entity *entity;
    vector_3d vel;
    int log_timer;
    bool did_collide, detonated, fire;
    std::vector< std::tuple< vector_3d, bool, vector_3d > > current_path, saved_path;
    int weapon_id;
    float spawn_time;
    float detonate_time;
    float curtime;
    int bounces;
    bool detonate;
};

struct nade_event_t {
    nade_event_t( int index, float time ) {
        this->index = index;
        this->time = time;
    }

    int index;
    float time;
};

inline std::vector< nade_event_t > g_nade_events = { };