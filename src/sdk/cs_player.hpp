#pragma once
#include "base_entity.hpp"
#include <globals.hpp>

class c_csgo_player_animstate;

struct c_animation_layer {
    PAD( 20 );
    int order;
    int sequence;
    float previous_cycle;
    float weight;
    float weight_delta_rate;
    float playback_rate;
    float cycle;
    void *owner;
    PAD( 4 );
};

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
    OFFSET( buttons, int, 0x31E8 );
    OFFSET( cstudio_hdr, c_studio_hdr *, 0x293C );
    NETVAR( immunity, bool, "DT_CSPlayer", "m_bGunGameImmunity" );
    NETVAR( duck_amount, float, "DT_CSPlayer", "m_flDuckAmount" );
    NETVAR( ducked, bool, "DT_CSPlayer", "m_bDucked" );
    NETVAR( ducking, bool, "DT_CSPlayer", "m_bDucking" );
    NETVAR( in_duck_jump, bool, "DT_CSPlayer", "m_bInDuckJump" );
    NETVAR( max_speed, float, "DT_CSPlayer", "m_flMaxspeed" );
    NETVAR( owner, uint32_t, "DT_CSPlayer", "m_hOwnerEntity" );

    void *get_view_model( ) {
        return g_interfaces.entity_list->get_client_entity_from_handle< void * >( this->viewmodel_handle( ) );
    }

    std::array< float, 24 > &pose_parameters( ) {
        static auto offset = g_netvars.get_offset( HASH_CT( "DT_BaseAnimating" ), HASH_CT( "m_flPoseParameter" ) );

        return *reinterpret_cast< std::array< float, 24 > * >( reinterpret_cast< uintptr_t >( this ) + offset );
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
    int lookup_sequence( const char *label );
    int get_sequence_activity( int sequence );
    void invalidate_bone_cache( );
    bool can_attack( );
    bool get_aim_matrix( vector_3d angle, matrix_3x4 *bones );
    void update_collision_bounds( );
    vector_3d get_shoot_position( );
};

class c_base_cs_grenade : public c_cs_player {
public:
NETVAR( redraw, bool, "DT_BaseCSGrenade", "m_bRedraw" );
    NETVAR( is_held_by_player, bool, "DT_BaseCSGrenade", "m_bIsHeldByPlayer" );
    NETVAR( pin_pulled, bool, "DT_BaseCSGrenade", "m_bPinPulled" );
    NETVAR( throw_time, float, "DT_BaseCSGrenade", "m_fThrowTime" );
    NETVAR( throw_strength, float, "DT_BaseCSGrenade", "m_flThrowStrength" );
};