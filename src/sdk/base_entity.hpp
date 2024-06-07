#pragma once
#include "base_player.hpp"
#include "interfaces/collideable.hpp"
#include <globals.hpp>

enum class move_types : uint8_t {
    none = 0,
    isometric,
    walk,
    step,
    fly,
    flygravity,
    vphysics,
    push,
    noclip,
    ladder,
    observer,
    custom
};

enum invalidate_physics_bits : int {
    position_changed = 0x1,
    angles_changed = 0x2,
    velocity_changed = 0x4,
    animation_changed = 0x8,
};

class var_map_entry_t {
public:
    unsigned short type;
    unsigned short needs_to_interpolate;
    void *data;
    void *watcher;
};

struct var_mapping_t {
    c_utl_vector< var_map_entry_t > entries;
    int interpolated_entries;
    float last_interpolation_time;
};

enum hitboxes : int {
    hitbox_head = 0,
    hitbox_neck,
    hitbox_lower_neck,
    hitbox_pelvis,
    hitbox_body,
    hitbox_thorax,
    hitbox_chest,
    hitbox_upper_chest,
    hitbox_r_thigh,
    hitbox_l_thigh,
    hitbox_r_calf,
    hitbox_l_calf,
    hitbox_r_foot,
    hitbox_l_foot,
    hitbox_r_hand,
    hitbox_l_hand,
    hitbox_r_upper_arm,
    hitbox_r_forearm,
    hitbox_l_upper_arm,
    hitbox_l_forearm,
    hitbox_max
};

//enum hitgroups : int {
//    hitgroup_generic = 0,
//    hitgroup_head,
//    hitgroup_chest,
//    hitgroup_stomach,
//    hitgroup_leftarm,
//    hitgroup_rightarm,
//    hitgroup_leftleg,
//    hitgroup_rightleg,
//    hitgroup_gear
//};

class c_base_entity : public c_base_player {
public:
    DATAMAP( move_type, move_types, this->get_pred_desc_map( ), "m_MoveType" );
    OFFSET( think_tick, int, 0xF8 );
    NETVAR( model_index, int, "DT_BaseEntity", "m_nModelIndex" );
    NETVAR( hitbox_set, int, "DT_BaseAnimatingOverlay", "m_nHitboxSet" );
    NETVAR( simtime, float, "DT_BaseEntity", "m_flSimulationTime" );
    NETVAR( old_simtime, float, "DT_BaseEntity", "m_flSimulationTime", +0x4 );
    NETVAR( ground_entity_handle, uint32_t, "DT_BasePlayer", "m_hGroundEntity" );
    NETVAR( model_scale, float, "DT_BaseAnimating", "m_flModelScale" );
    OFFSET( bone_count, int, 0x2908 );
    OFFSET( bone_cache, matrix_3x4 *, 0x28FC );

    bool is_player( ) {
        return utils::get_method< bool( __thiscall * )( void * ) >( this, 152 )( this );
    }

    var_mapping_t *get_var_mapping( ) {
        return reinterpret_cast< var_mapping_t * >( reinterpret_cast< uintptr_t >( this ) + 0x14 );
    }

    vector_3d &mins( ) {
        auto offset = g_netvars.get_offset( HASH_CT( "DT_BaseEntity" ), HASH_CT( "m_Collision" ) );

        return *reinterpret_cast< vector_3d * >( reinterpret_cast< uintptr_t >( this ) + offset + g_netvars.get_offset( HASH_CT( "DT_BaseEntity" ), HASH_CT( "m_vecMins" ) ) );
    }

    vector_3d &maxs( ) {
        auto offset = g_netvars.get_offset( HASH_CT( "DT_BaseEntity" ), HASH_CT( "m_Collision" ) );

        return *reinterpret_cast< vector_3d * >( reinterpret_cast< uintptr_t >( this ) + offset + g_netvars.get_offset( HASH_CT( "DT_BaseEntity" ), HASH_CT( "m_vecMaxs" ) ) );
    }

    vector_3d world_space_center( ) {
        const auto collideable = this->collideable( );

        if ( !collideable )
            return { };

        return this->origin( ) + ( ( collideable->mins( ) + collideable->maxs( ) ) * 0.5f );
    }

    bool get_hitbox_position( const int hit_group, vector_3d &position );
    void set_sequence( int sequence );
    void set_abs_origin( const vector_3d &origin );
    void invalidate_physics_recursive( int change_flags );
    void set_abs_angles( const vector_3d &angles );
    void set_collision_bounds( const vector_3d &mins, const vector_3d &maxs );
    void invalidate_bone_cache( );
    bool physics_run_think( int think_method = 0 );
    void set_next_think( int context_index, float think_time = 0 );
    vector_3d get_bone_position( const int bone, matrix_3x4 *bone_matrix );
    c_base_entity *get_root_move_parent( );
    c_base_entity *get_move_parent( );
    void *get_predicted_frame( int framenumber );
};

class c_view_model : public c_base_entity {
public:
    NETVAR( model_index, int, "DT_BaseViewModel", "m_nModelIndex" );
    NETVAR( sequence, int, "DT_BaseViewModel", "m_nSequence" );
    NETVAR( animation_parity, int, "DT_BaseViewModel", "m_nAnimationParity" );
    NETVAR( weapon, int, "DT_BaseViewModel", "m_hWeapon" );
};