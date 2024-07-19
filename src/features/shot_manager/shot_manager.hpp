#pragma once
#include <globals.hpp>
#include <features/animations/animation_sync.hpp>

struct shot_record_t {
public:
    shot_record_t( ) : target{ }, record{ }, time{ }, lat{ }, damage{ }, pos{ }, server_pos{ }, matched{ }, impacted{ }, hurt_player{ } {}

public:
    c_cs_player *target;
    lag_record *record;
    float time, lat, damage;
    int lag;
    vector_3d pos;
    vector_3d server_pos;
    bool impacted;
    bool matched;
    bool hurt_player;
};

struct impact_record_t {
public:
    impact_record_t( ) : shot{ }, pos{ }, tick{ } {}

public:
    shot_record_t *shot;
    int tick;
    vector_3d pos;
};

struct hit_record_t {
public:
    hit_record_t( ) : impact{ }, group{ -1 }, damage{ } {}

public:
    impact_record_t *impact;
    int group;
    float damage;
};

struct visual_impact_data_t {
public:
    vector_3d impact_pos, shoot_pos;
    int tickbase;
    bool ignore, hurt_player;

public:
    visual_impact_data_t( const vector_3d &impact_pos, const vector_3d &shoot_pos, int tickbase ) : impact_pos{ impact_pos }, shoot_pos{ shoot_pos }, tickbase{ tickbase }, ignore{ false }, hurt_player{ false } {}
};

class shot_manager {
public:
    std::array< std::string, 8 > hitgroup_names = {
            _xs( "body" ),
            _xs( "head" ),
            _xs( "chest" ),
            _xs( "stomach" ),
            _xs( "left arm" ),
            _xs( "right arm" ),
            _xs( "left leg" ),
            _xs( "right leg" )
    };

	void on_shot_fire( c_cs_player *target, float damage, int bullets, lag_record *record );
    void reset_data( );
    void on_round_start( event_t *evt );
    void process_shots( );
    void on_impact( event_t *evt );
    void on_hurt( event_t *evt );

public:
    std::deque< visual_impact_data_t > vis_impacts;
	std::deque< shot_record_t > shots;
	std::deque< hit_record_t > hits;
    std::deque< impact_record_t > impacts;
};

inline shot_manager g_shot_manager;