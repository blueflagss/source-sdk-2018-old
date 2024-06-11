#pragma once

#include "utils/netvars/netvars.hpp"
#include "utils/datamaps/datamaps.hpp"
#include <globals.hpp>
#include <sdk/math/aligned_vector.hpp>
#include <sdk/user_cmd.hpp>
#include <utils/math/math.hpp>

enum player_flags : int {
    on_ground = ( 1 << 0 ),
    ducking = ( 1 << 1 ),
    water_jump = ( 1 << 2 ),
    on_train = ( 1 << 3 ),
    in_rain = ( 1 << 4 ),
    frozen = ( 1 << 5 ),
    at_controls = ( 1 << 6 ),
    client = ( 1 << 7 ),
    fake_client = ( 1 << 8 ),
    in_water = ( 1 << 9 )
};

enum hitgroups : int {
    hitgroup_generic = 0,
    hitgroup_head,
    hitgroup_chest,
    hitgroup_stomach,
    hitgroup_leftarm,
    hitgroup_rightarm,
    hitgroup_leftleg,
    hitgroup_rightleg,
    hitgroup_gear
};

enum life_states : char {
    alive = 0,
    dying,
    dead,
    respawnable,
    discardbody
};

class c_mouth_info;

class i_client_entity : public i_client_unknown, public i_client_renderable, public i_client_networkable, public i_client_thinkable {
public:
    virtual void release( void ) = 0;
    virtual const vector_3d &get_abs_origin( void ) const = 0;
    virtual const vector_3d &get_abs_angles( void ) const = 0;
    virtual c_mouth_info *get_mouth( void ) = 0;
    virtual bool get_sound_spatialization( uint16_t &info ) = 0;

    datamap_t *get_pred_desc_map( ) {
        return utils::get_method< datamap_t *( __thiscall * ) ( void * ) >( this, 17 )( this );
    }
};

class c_player_state {
public:
    virtual ~c_player_state( ) {}

    bool deadflag;
    vector_3d v_angle;

    const char *netname;

    int fix_angle;
    vector_3d angle_change;

    bool hltv;
    bool replay;
    int frags;
    int deaths;
};

class c_base_player : public i_client_entity {
public:
    template< typename T >
    __inline T get( ) {
        return reinterpret_cast< T >( this );
    }

public:
    NETVAR( rgfl_coordinate_frame, matrix_3x4, "DT_BaseEntity", "m_vecOrigin", -0x30 );

    DATAMAP( current_command, c_user_cmd *, this->get_pred_desc_map( ), "m_nButtons", +0x4 );
    DATAMAP( eflags, int, this->get_pred_desc_map( ), "m_iEFlags" );
    DATAMAP( impulse, unsigned char, this->get_pred_desc_map( ), "m_nImpulse" );

    NETVAR( team, int, "DT_BasePlayer", "m_iTeamNum" );
    NETVAR( origin, vector_3d, "DT_BaseEntity", "m_vecOrigin" );
    NETVAR( health, int, "DT_BasePlayer", "m_iHealth" );
    NETVAR( fov, int, "DT_BasePlayer", "m_iFOV" );
    NETVAR( sequence, int, "DT_BasePlayer", "m_nSequence" );
    NETVAR( flags, int, "DT_BasePlayer", "m_fFlags" );
    NETVAR( velocity, vector_3d, "DT_CSPlayer", "m_vecVelocity[0]" );
    NETVAR( fall_velocity, float, "DT_BasePlayer", "m_flFallVelocity" );
    NETVAR( life_state, char, "DT_BasePlayer", "m_lifeState" );
    NETVAR( tick_base, int, "DT_BasePlayer", "m_nTickBase" );
    OFFSET( final_predicted_tick, int, 0x1258 );
    NETVAR( ground_entity, int, "DT_BasePlayer", "m_hGroundEntity" );
    NETVAR( base_velocity, vector_3d, "DT_BasePlayer", "m_vecBaseVelocity" );
    NETVAR( localdata, void *, "DT_BasePlayer", "localdata" );
    NETVAR( view_offset, vector_3d, "DT_BasePlayer", "m_vecViewOffset[0]" );
    NETVAR( view_punch, vector_3d, "DT_BasePlayer", "m_viewPunchAngle" );
    NETVAR( aim_punch, vector_3d, "DT_BasePlayer", "m_aimPunchAngle" );
    NETVAR( weapon_handle, uint32_t, "DT_BaseCombatCharacter", "m_hActiveWeapon" );
    NETVAR( next_attack, float, "DT_BaseCombatCharacter", "m_flNextAttack" );
    NETVAR( player_state, c_player_state, "DT_BasePlayer", "pl" );

    bool compute_bounding_box( box &box_dimensions );
    bool is_player_on_steam_friends( ) const;
    void update_clientside_animation( );
    void select_item( const char *str, int subtype );
    void update_button_state( int button_mask );
    void pre_think( );
    void think( );
    void post_think( );

    inline bool alive( ) {
        return this->life_state( ) == life_states::alive && this->health( ) > 0;
    }

    vector_3d world_space_center( );
};

class c_base_view_model : public c_base_player {
public:
    NETVAR( sequence, int, "DT_BaseViewModel", "m_nSequence" );

    c_base_player *owner( ) {
        return utils::get_method< c_base_player *( __thiscall * ) ( void * ) >( this, 214 )( this );
    }
};