#include "bone_setup.hpp"

void bone_setup::build( c_cs_player *pl, matrix_3x4 *bone_to_world, int mask, const vector_3d &origin, const vector_3d &angles, float time, const vector_3d &eye_angles ) {
    auto cstudio_hdr = pl->cstudio_hdr( );

    if ( !cstudio_hdr )
        return;

    auto accessor = &pl->bone_accessor( );

    if ( !accessor )
        return;

    const auto backup_matrix = accessor->m_pBones;

    if ( !backup_matrix )
        return;

    matrix_3x4_aligned tmp[ 128 ];
    accessor->m_pBones = tmp;

    mask |= bone_always_setup;
    mask &= ~( bone_used_by_bone_merge );

    globals::is_building_bones[ pl->index( ) ] = true;

    if ( pl == globals::local_player )
        pl->anim_overlays( )[ 12 ].weight = 0.f;

    static auto iks_off = signature::find( _xs( "client.dll" ), _xs( "8D 47 FC 8B 8F" ) ).add( 5 ).deref( ).add( 4 ).get< uint32_t >( );
    static auto effects_off = signature::find( _xs( "client.dll" ), _xs( "75 0D 8B 87" ) ).add( 4 ).deref( ).add( 4 ).get< uint32_t >( );

    uint8_t bone_computed[ 0x100 ] = { 0 };

    alignas( 16 ) vector_3d pos[ 128 ];
    alignas( 16 ) vector_4d q[ 128 ];

    ZeroMemory( pos, sizeof( vector_3d[ 128 ] ) );
    ZeroMemory( q, sizeof( vector_4d[ 128 ] ) );

    const auto backup_bone_setup_time = pl->last_bone_setup_time( );
    const auto backup_effects = pl->effects( );
    const auto backup_ik = pl->ik_context( );
    const auto backup_abs_angles = pl->get_abs_angles( );
    const auto backup_eye_angle = pl->eye_angles( );
    const auto backup_view_offset = pl->view_offset( );
    const auto backup_lod_flags = pl->anim_lod_flags( );
    const auto backup_time = g_interfaces.global_vars->curtime;

    pl->effects( ) |= effects::nointerp;
    pl->is_jiggle_bones_enabled( ) = false;
    pl->anim_lod_flags( ) = 0;
    pl->eye_angles( ) = eye_angles;
    pl->last_bone_setup_time( ) = 0;

    g_interfaces.global_vars->curtime = time;

    c_ik_context ik;
    ik.clear_targets( );
    ik.init( cstudio_hdr, angles, origin, time, g_interfaces.global_vars->framecount, mask );
    pl->ik_context( ) = &ik;

    pl->standard_blending_rules( cstudio_hdr, pos, q, time, mask );
    pl->update_ik_locks( time );
    ik.update_targets( pos, q, accessor->m_pBones, bone_computed );
    pl->calculate_ik_locks( time );
    ik.solve_dependencies( pos, q, accessor->m_pBones, bone_computed );

    matrix_3x4_aligned rot_matrix;                     
    math::angle_matrix( angles, rot_matrix );       
    rot_matrix.set_origin( origin );

    pl->build_transformations( cstudio_hdr, pos, q, rot_matrix, mask, bone_computed );
    std::memcpy( bone_to_world, accessor->m_pBones, sizeof( matrix_3x4 ) * 128 );
    accessor->m_pBones = backup_matrix;

    pl->eflags( ) &= ~0x003;
    pl->set_abs_angles( backup_abs_angles );
    pl->eye_angles( ) = backup_eye_angle;
    pl->effects( ) = backup_effects;
    pl->ik_context( ) = backup_ik;
    pl->view_offset( ) = backup_view_offset;
    pl->anim_lod_flags( ) = backup_lod_flags;
    pl->last_bone_setup_time( ) = backup_bone_setup_time;
    g_interfaces.global_vars->curtime = backup_time;

    globals::is_building_bones[ pl->index( ) ] = false;
}