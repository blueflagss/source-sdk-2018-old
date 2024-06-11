#pragma once
#include "base_player.hpp"
#include "interfaces/collideable.hpp"
#include <globals.hpp>

struct mstudioactivitymodifier_t {
    int sznameindex;

    inline char *name( ) { return ( sznameindex ) ? ( char * ) ( ( ( char * ) this ) + sznameindex ) : NULL; }
};

struct mstudioanimtag_t {
    int tag;
    float cycle;
    int sztagindex;
};

struct mstudioseqdesc_t {
    int baseptr;
    inline studiohdr_t *studio_hdr( void ) const { return ( studiohdr_t * ) ( ( ( char * ) this ) + baseptr ); }
    int szlabelindex;
    inline char *const label( void ) const { return ( ( char * ) this ) + szlabelindex; }
    int szactivitynameindex;
    inline char *const activity_name( void ) const { return ( ( char * ) this ) + szactivitynameindex; }
    int flags;
    int activity;
    int actweight;
    int numevents;
    int eventindex;
    vector_3d bbmin;    
    vector_3d bbmax;
    int numblends;
    int animindexindex;

    inline int anim( int x, int y ) const {
        if ( x >= groupsize[ 0 ] ) {
            x = groupsize[ 0 ] - 1;
        }

        if ( y >= groupsize[ 1 ] ) {
            y = groupsize[ 1 ] - 1;
        }

        int offset = y * groupsize[ 0 ] + x;
        short *blends = ( short * ) ( ( ( char * ) this ) + animindexindex );
        int value = ( int ) blends[ offset ];
        return value;
    }

    int movementindex;      
    int groupsize[ 2 ];
    int paramindex[ 2 ];        
    float paramstart[ 2 ];    
    float paramend[ 2 ];      
    int paramparent;
    float fadeintime;        
    float fadeouttime;      
    int localentrynode;    
    int localexitnode;     
    int nodeflags;       
    float entryphase;     
    float exitphase;      
    float lastframe;     
    int nextseq;   
    int pose;           
    int numikrules;
    int numautolayers;
    int autolayerindex;
    int weightlistindex;
    int posekeyindex;
    int numiklocks;
    int iklockindex;
    int keyvalueindex;
    int keyvaluesize;
    int cycleposeindex;
    int activitymodifierindex;
    int numactivitymodifiers;

    inline mstudioactivitymodifier_t *activity_modifier( int i ) const {
        return activitymodifierindex != 0 ? ( mstudioactivitymodifier_t * ) ( ( ( char * ) this ) + activitymodifierindex ) + i : NULL;
    };

    int animtagindex;
    int numanimtags;

    inline mstudioanimtag_t *anim_tag( int i ) const {
        return ( mstudioanimtag_t * ) ( ( ( char * ) this ) + animtagindex ) + i;
    };

    int rootDriverIndex;
    int unused[ 2 ];           

    mstudioseqdesc_t( ) {}

private:
    mstudioseqdesc_t( const mstudioseqdesc_t &vOther );
};

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

enum csgo_hitbox : int {
    hitbox_head = 0,
    hitbox_neck,
    //hitbox_lower_neck,
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

inline static const csgo_hitbox player_hitboxes[] = {
        hitbox_head, hitbox_neck, hitbox_pelvis, hitbox_body,
        hitbox_thorax, hitbox_chest, hitbox_upper_chest, hitbox_r_thigh,
        hitbox_l_thigh, hitbox_r_calf, hitbox_l_calf, hitbox_r_foot,
        hitbox_l_foot, hitbox_r_hand, hitbox_l_hand, hitbox_r_upper_arm,
        hitbox_r_forearm, hitbox_l_upper_arm, hitbox_l_forearm };

class c_base_entity : public c_base_player {
public:
    DATAMAP( move_type, move_types, this->get_pred_desc_map( ), "m_MoveType" );
    DATAMAP( abs_rotation, vector_3d, this->get_pred_desc_map( ), "m_angAbsRotation" );
    OFFSET( think_tick, int, 0xF8 );
    NETVAR( model_index, int, "DT_BaseEntity", "m_nModelIndex" );
    NETVAR( hitbox_set, int, "DT_BaseAnimatingOverlay", "m_nHitboxSet" );
    NETVAR( simtime, float, "DT_BaseEntity", "m_flSimulationTime" );
    NETVAR( animtime, float, "DT_BaseEntity", "m_flAnimTime" );
    NETVAR( old_simtime, float, "DT_BaseEntity", "m_flSimulationTime", +0x4 );
    NETVAR( ground_entity_handle, uint32_t, "DT_BasePlayer", "m_hGroundEntity" );
    NETVAR( cycle, float, "DT_BaseAnimating", "m_flCycle" );
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
    bool physics_run_think( int think_method = 0 );
    void set_next_think( int context_index, float think_time = 0 );
    void attachment_helper( );
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