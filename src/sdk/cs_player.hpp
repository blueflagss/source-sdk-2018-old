#pragma once
#include "base_entity.hpp"
#include <globals.hpp>

#define bone_calculate_mask 0x1f
#define bone_physically_simulated 0x01
#define bone_physics_procedural 0x02
#define bone_always_procedural 0x04
#define bone_screen_align_sphere 0x08
#define bone_screen_align_cylinder 0x10
#define bone_world_align 0x20
#define bone_used_mask 0x000fff00
#define bone_used_by_anything 0x000fff00
#define bone_used_by_hitbox 0x00000100
#define bone_used_by_attachment 0x00000200
#define bone_used_by_vertex_mask 0x0003fc00
#define bone_used_by_vertex_lod0 0x00000400
#define bone_used_by_vertex_lod1 0x00000800
#define bone_used_by_vertex_lod2 0x00001000
#define bone_used_by_vertex_lod3 0x00002000
#define bone_used_by_vertex_lod4 0x00004000
#define bone_used_by_vertex_lod5 0x00008000
#define bone_used_by_vertex_lod6 0x00010000
#define bone_used_by_vertex_lod7 0x00020000
#define bone_used_by_bone_merge 0x00040000
#define bone_always_setup 0x00080000

class c_csgo_player_animstate;

enum PlayerFlags_t : int {
    FL_ONGROUND = ( 1 << 0 ),
    FL_DUCKING = ( 1 << 1 ),
    FL_WATERJUMP = ( 1 << 3 ),
    FL_ONTRAIN = ( 1 << 4 ),
    FL_INRAIN = ( 1 << 5 ),
    FL_FROZEN = ( 1 << 6 ),
    FL_ATCONTROLS = ( 1 << 7 ),
    FL_CLIENT = ( 1 << 8 ),
    FL_FAKECLIENT = ( 1 << 9 ),
    FL_INWATER = ( 1 << 10 ),
};

enum effects : int {
    bonemerge = 0x001,
    brightlight = 0x002,
    dimlight = 0x004,
    nointerp = 0x008,
    noshadow = 0x010,
    nodraw = 0x020,
    noreceiveshadow = 0x040,
    bonemerge_fastcull = 0x080,
    item_blink = 0x100,
    parent_animates = 0x200
};

enum pose_param : int {
    strafe_yaw = 0,
    stand,
    lean_yaw,
    _speed,
    ladder_yaw,
    ladder_speed,
    jump_fall,
    move_yaw,
    move_blend_crouch,
    move_blend_walk,
    move_blend_run,
    body_yaw,
    body_pitch,
    aim_blend_stand_idle,
    aim_blend_stand_walk,
    aim_blend_stand_run,
    aim_blend_courch_idle,
    aim_blend_crouch_walk,
    death_yaw
};

class c_base_attribute_item;

class c_bone_accessor {
public:
    void *m_pAnimating;
    matrix_3x4_aligned *m_pBones;
    int m_ReadableBones;
    int m_WritableBones;
};

class C_BoneMergeCache {
public:
    __forceinline void init( void *owner ) {
        static auto init = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 8B 45 08 56 8B F1 89 06 C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C6 86 ? ? ? ? ? C7 86 ? ? ? ? ? ? ? ? 83 BE" ) ).get< void( __thiscall * )( void *, void * ) >( );
        init( this, owner );
    }

    void *operator new( size_t size ) {
        static auto C_BoneMergeCache__Constructor = signature::find( _xs( "client.dll" ), _xs( "56 8B F1 0F 57 C0 C7 86 ?? ?? ?? ?? ?? ?? ?? ?? C7 86 ?? ?? ?? ?? ?? ?? ?? ?? C7 86" ) ).get< void( __thiscall * )( void * ) >( );

        C_BoneMergeCache *ptr = ( C_BoneMergeCache * ) g_interfaces.mem_alloc->alloc( size );
        C_BoneMergeCache__Constructor( ptr );

        return ptr;
    }

    void operator delete( void *ptr ) {
        g_interfaces.mem_alloc->free( ptr );
    }

    __forceinline void copy_from_follow( vector_3d *followPos, const vector_4d followQ[], int boneMask, vector_3d myPos[ 256 ], vector_4d myQ[ 256 ] ) {
        static auto copy_from_follow = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 08 53 56 57 8B F9 89 7D F8 E8 ? ? ? ? 83 7F 10 00 0F 84 ? ? ? ? 8B 87 ? ? ? ? 85 C0 74 75" ) ).get< void( __thiscall * )( void *, vector_3d *, const vector_4d[], int, vector_3d[ 256 ], vector_4d[ 256 ] ) >( );
        copy_from_follow( this, followPos, followQ, boneMask, myPos, myQ );
    }

    __forceinline void copy_to_follow( vector_3d *myPos, const vector_4d myQ[], int boneMask, vector_3d followPos[ 256 ], vector_4d followQ[ 256 ] ) {
        static auto copy_to_follow = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 24 8B 45 08 57 8B F9 89 7D F4 85 C0 0F 84 ? ? ? ? 05 ? ? ? ? 53 56 89 45 EC" ) ).get< void( __thiscall * )( void *, vector_3d *, const vector_4d[], int, vector_3d[ 256 ], vector_4d[ 256 ] ) >( );
        copy_to_follow( this, myPos, myQ, boneMask, followPos, followQ );
    }

    __forceinline void merge_matching_pose_params( ) {
        static auto merge_matching_pose_params = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 0C 53 56 8B F1 57 89 75 F8 E8 ? ? ? ? 83 7E 10 00 0F 84 ? ? ? ? 83 BE" ) ).get< void( __thiscall * )( void * ) >( );
        merge_matching_pose_params( this );
    }

    void *m_pOwner;
    void *m_pFollow;
    c_studio_hdr *m_pFollowHdr;
    const studiohdr_t *m_pFollowRenderHdr;
    c_studio_hdr *m_pOwnerHdr;
    const studiohdr_t *m_pOwnerRenderHdr;
    int m_nCopiedFramecount;
    int m_nFollowBoneSetupMask;

    class CMergedBone {
    public:
        unsigned short m_iMyBone;
        unsigned short m_iParentBone;
    };

    int m_nOwnerToFollowPoseParamMapping[ 24 ];
    c_utl_vector< CMergedBone > m_MergedBones;
    PAD( 12U );
    unsigned short m_iRawIndexMapping[ 138 ];
    bool m_bForceCacheClear;
};

class c_ik_context;
class c_base_animating;

class c_cs_player : public c_base_entity {
public:
    OFFSET( button_forced, int, 0x3310 );
    OFFSET( spawn_time, float, 0xA290 );
    OFFSET( button_pressed, int, 0x31E0 );
    OFFSET( is_jiggle_bones_enabled, int, 0x291C );
    OFFSET( button_last, int, 0x31DC );
    OFFSET( move_state, int, 0x38D0 );
    OFFSET( anim_state, c_csgo_player_animstate *, 0x3874 );
    OFFSET( button_released, int, 0x31E4 );
    OFFSET( button_disabled, int, 0x3340 );
    OFFSET( maintain_sequence_transitions, bool, 0x9F0 );
    OFFSET( last_bone_setup_time, float, 0x2910 );
    OFFSET( use_new_animstate, bool, 0x39E1 );
    NETVAR( survival_team, int, "DT_CSPlayer", "m_nSurvivalTeam" );
    NETVAR( account, int, "DT_CSPlayer", "m_iAccount" );
    NETVAR( scoped, bool, "DT_CSPlayer", "m_bIsScoped" );
    NETVAR( armor, int, "DT_CSPlayer", "m_ArmorValue" );
    NETVAR( encoded_controller, float *, "DT_CSPlayer", "m_flEncodedController" );
    NETVAR( helmet, bool, "DT_CSPlayer", "m_bHasHelmet" );
    NETVAR( effects, int, "DT_CSPlayer", "m_fEffects" );
    NETVAR( velocity_modifier, float, "DT_CSPlayer", "m_flVelocityModifier" );
    NETVAR( heavy_armor, bool, "DT_CSPlayer", "m_bHasHeavyArmor" );
    NETVAR( client_side_animation, bool, "DT_BaseAnimating", "m_bClientSideAnimation" );
    NETVAR( strafing, bool, "DT_CSPlayer", "m_bStrafing" );
    NETVAR( flash_duration, float, "DT_CSPlayer", "m_flFlashDuration" );
    NETVAR( eye_angles, vector_3d, "DT_CSPlayer", "m_angEyeAngles[0]" );
    NETVAR( lower_body_yaw_target, float, "DT_CSPlayer", "m_flLowerBodyYawTarget" );
    NETVAR( viewmodel_handle, uint32_t, "DT_BasePlayer", "m_hViewModel[0]" );
    OFFSET( current_cmd, c_user_cmd *, 0x3348 );
    OFFSET( abs_velocity, vector_3d, 0x94 );
    OFFSET( last_cmd, c_user_cmd, 0x3298 );
    OFFSET( anim_overlays, c_animation_layer *, 0x2970 );
    OFFSET( num_overlays, int, 0x297C );
    OFFSET( buttons, int, 0x31E8 );
    OFFSET( cstudio_hdr, c_studio_hdr *, 0x293C );
    NETVAR( immunity, bool, "DT_CSPlayer", "m_bGunGameImmunity" );
    NETVAR( ducked, bool, "DT_CSPlayer", "m_bDucked" );
    NETVAR( ducking, bool, "DT_CSPlayer", "m_bDucking" );
    NETVAR( in_duck_jump, bool, "DT_CSPlayer", "m_bInDuckJump" );
    NETVAR( max_speed, float, "DT_CSPlayer", "m_flMaxspeed" );
    NETVAR( owner, uint32_t, "DT_CSPlayer", "m_hOwnerEntity" );
    OFFSET( bone_merge_cache, C_BoneMergeCache *, 0x28FC );
    NETVAR( observer_target, uint32_t, "DT_CSPlayer", "m_hObserverTarget" );

    c_bone_accessor &bone_accessor( ) {
        static auto offset = signature::find( _xs( "client.dll" ), _xs( "8D 81 ? ? ? ? 50 8D 84 24" ) ).add( 0x2 ).deref( ).get< uintptr_t >( );

        return *reinterpret_cast< c_bone_accessor * >( reinterpret_cast< uintptr_t >( this ) + offset );
    }

    c_base_view_model *get_view_model( ) {
        if ( !this || this->viewmodel_handle( ) == -1 )
            return nullptr;

        return g_interfaces.entity_list->get_client_entity_from_handle< c_base_view_model * >( this->viewmodel_handle( ) );
    }

    c_cs_player *get_observer_target( ) {
        return g_interfaces.entity_list->get_client_entity_from_handle< c_cs_player * >( this->observer_target( ) );
    }

    std::array< float, 24 > &pose_parameters( ) {
        static auto offset = g_netvars.get_offset( HASH_CT( "DT_BaseAnimating" ), HASH_CT( "m_flPoseParameter" ) );

        return *reinterpret_cast< std::array< float, 24 > * >( reinterpret_cast< uintptr_t >( this ) + offset );
    }

    c_base_animating *get_base_animating( ) {
        return utils::get_method< c_base_animating *( __thiscall * ) ( void * ) >( this, 44 )( this );
    }

    c_ik_context *&ik_context( ) {
        return *reinterpret_cast< c_ik_context ** >( reinterpret_cast< uintptr_t >( this ) + 0x265C );
    }

    studiohdr_t *get_model_ptr( );
    float get_first_sequence_anim_tag( int sequence, int desired_tag, float start, float end );
    void get_sequence_linear_motion( void *hdr, int seq, const float poses[], vector_3d *vec );
    float get_sequence_move_dist( void *hdr, int seq );
    int lookup_pose_parameter( const char *name );
    float set_pose_parameter( int index, float value );
    float sequence_duration( int sequence );
    float get_sequence_cycle_rate( int sequence );
    float get_pose_parameter( int index );
    void get_bone_position( int bone, vector_3d &out, vector_3d &q );
    int lookup_bone( const char *name );
    void modify_eye_position( c_csgo_player_animstate *state, vector_3d *input_eye_pos, matrix_3x4 *bones );
    int &anim_lod_flags( );
    int lookup_sequence( const char *label );
    int get_sequence_activity( int sequence );
    void invalidate_bone_cache( );
    bool can_attack( );
    bool get_aim_matrix( vector_3d angle, matrix_3x4 *bones );
    void update_collision_bounds( );
    void standard_blending_rules( c_studio_hdr *hdr, vector_3d *pos, vector_4d *q, float time, int mask );
    void build_transformations( c_studio_hdr *hdr, vector_3d *pos, vector_4d *q, const matrix_3x4_aligned &mat, int mask, uint8_t *computed );
    void update_ik_locks( float time );
    std::deque< c_base_attribute_item * > weapons( );
    vector_3d get_shoot_position( );
};

class c_base_animating : public c_cs_player {
public:
};