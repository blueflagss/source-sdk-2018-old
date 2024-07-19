#pragma once
#include <globals.hpp>

class grenade_prediction {
public:
    std::vector< std::tuple< vector_3d, bool, vector_3d > > path;
        
    vector_3d origin;
    int last_update_tick;

	bool PhysicsCheckWater( c_cs_player *player );

    void PhysicsRunThink( c_cs_player *player );

    void UpdatePath( bool Bounced );

    int physics_clip_velocity( const vector_3d &in, const vector_3d &normal, vector_3d &out, float overbounce );

    void push_entity( c_cs_player *player, vector_3d &angle );

    void ResolveFlyCollisionCustom( c_game_trace &trace, vector_3d &vecVelocity, float interval );

    void render( );

    void simulate_path( c_cs_player *player, vector_3d &vecSrc );

    void PhysicsAddGravityMove( c_cs_player *player, vector_3d &move );

    bool check_detonate( const vector_3d &vecThrow, const c_game_trace &tr, int tick, float interval );

    int tick( c_cs_player *player, vector_3d &src, vector_3d &vec_throw, int tick, float interval );

    int log_timer;
    bool did_collide, detonated, fire;
};

inline grenade_prediction g_grenade_prediction = { };