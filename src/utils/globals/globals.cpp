#include "globals.hpp"

void globals::cvars::init( ) {
    cl_predict = g_interfaces.cvar->find_var( HASH_CT( "cl_predict" ) );
    cl_extrapolate = g_interfaces.cvar->find_var( HASH_CT( "cl_extrapolate" ) );
  
    cl_extrapolate->remove_callbacks( );
    cl_extrapolate->set_int( 0 );

    cl_interp_ratio = g_interfaces.cvar->find_var( HASH_CT( "cl_interp_ratio" ) );
    cl_updaterate = g_interfaces.cvar->find_var( HASH_CT( "cl_updaterate" ) );
    weapon_accuracy_shotgun_spread_patterns = g_interfaces.cvar->find_var( HASH_CT( "weapon_accuracy_shotgun_spread_patterns" ) );
    sv_minupdaterate = g_interfaces.cvar->find_var( HASH_CT( "sv_minupdaterate" ) );
    sv_maxupdaterate = g_interfaces.cvar->find_var( HASH_CT( "sv_maxupdaterate" ) );
    sv_client_min_interp_ratio = g_interfaces.cvar->find_var( HASH_CT( "sv_client_min_interp_ratio" ) );
    sv_client_max_interp_ratio = g_interfaces.cvar->find_var( HASH_CT( "sv_client_max_interp_ratio" ) );
    cl_sidespeed = g_interfaces.cvar->find_var( HASH_CT( "cl_sidespeed" ) );
    weapon_recoil_scale = g_interfaces.cvar->find_var( HASH_CT( "weapon_recoil_scale" ) );
    cl_forwardspeed = g_interfaces.cvar->find_var( HASH_CT( "cl_forwardspeed" ) );
    cl_upspeed = g_interfaces.cvar->find_var( HASH_CT( "cl_upspeed" ) );
    sv_maxunlag = g_interfaces.cvar->find_var( HASH_CT( "sv_maxunlag" ) );
    cl_lagcompensation = g_interfaces.cvar->find_var( HASH_CT( "cl_lagcompensation" ) );
    cl_interp = g_interfaces.cvar->find_var( HASH_CT( "cl_interp" ) );
    sv_maxusrcmdprocessticks = g_interfaces.cvar->find_var( HASH_CT( "sv_maxusrcmdprocessticks" ) );
    mp_damage_scale_ct_body = g_interfaces.cvar->find_var( HASH_CT( "mp_damage_scale_ct_body" ) );
    mp_damage_scale_t_body = g_interfaces.cvar->find_var( HASH_CT( "mp_damage_scale_t_body" ) );
    mp_damage_scale_ct_head = g_interfaces.cvar->find_var( HASH_CT( "mp_damage_scale_ct_head" ) );
    mp_damage_scale_t_head = g_interfaces.cvar->find_var( HASH_CT( "mp_damage_scale_t_head" ) );
    sv_penetration_type = g_interfaces.cvar->find_var( HASH_CT( "sv_penetration_type" ) );
    ff_damage_reduction_bullets = g_interfaces.cvar->find_var( HASH_CT( "ff_damage_reduction_bullets" ) );
    ff_damage_bullet_penetration = g_interfaces.cvar->find_var( HASH_CT( "ff_damage_bullet_penetration" ) );
    sv_clip_penetration_traces_to_players = g_interfaces.cvar->find_var( HASH_CT( "sv_clip_penetration_traces_to_players" ) );

    cl_csm_shadows = g_interfaces.cvar->find_var( HASH_CT( "cl_csm_shadows" ) );
    cl_csm_shadows->remove_callbacks( );
    cl_csm_shadows->set_int( 0 );

    cl_foot_contact_shadows = g_interfaces.cvar->find_var( HASH_CT( "cl_foot_contact_shadows" ) );
    cl_foot_contact_shadows->remove_callbacks( );
    cl_foot_contact_shadows->set_int( 0 );

    name = g_interfaces.cvar->find_var( HASH_CT( "name" ) );
    name->remove_callbacks( );

    r_DrawSpecificStaticProp = g_interfaces.cvar->find_var( HASH_CT( "r_DrawSpecificStaticProp" ) );
    r_DrawSpecificStaticProp->remove_callbacks( );
}