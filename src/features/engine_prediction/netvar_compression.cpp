#include "netvar_compression.hpp"

void netvar_compression::init( c_cs_player *player ) {
    if ( initialized )
        return;

    ADD_NETVAR( float_vars, "DT_BasePlayer", "m_flFallVelocity", 1.f / assign_range_multiplier( 17, 4096.f - ( -4096.f ) ) );
    ADD_DATAMAP_VAR( float_vars, player->get_pred_desc_map( ), "m_flMaxspeed", 1.f / assign_range_multiplier( 12, 2048.0f ) );
    ADD_NETVAR( float_vars, "DT_BasePlayer", "m_vecViewOffset[0]", 1.f / assign_range_multiplier( 8, 32.0f - ( -32.0f ) ) );
    ADD_NETVAR( float_vars, "DT_BasePlayer", "m_vecViewOffset[1]", 1.f / assign_range_multiplier( 8, 32.0f - ( -32.0f ) ) );
    ADD_NETVAR( float_vars, "DT_BasePlayer", "m_vecViewOffset[2]", 1.f / assign_range_multiplier( 10, 128.f ) );
    ADD_NETVAR( vec3d_vars, "DT_BasePlayer", "m_vecBaseVelocity", 1.f / assign_range_multiplier( 20, 2000.f ) );
    ADD_NETVAR( vec3d_vars, "DT_BasePlayer", "m_vecVelocity[0]", 0.031250f );
    ADD_NETVAR( vec3d_vars, "DT_BasePlayer", "m_aimPunchAngle", 0.031250f );
    ADD_NETVAR( float_vars, "DT_CSPlayer", "m_flVelocityModifier", ( 1.f / assign_range_multiplier( 8.f, 1.f ) ) );

    initialized = true;
}

void netvar_compression::pre_update( c_cs_player *player ) {
    written_pre_vars = true;

    auto player_addr = reinterpret_cast< size_t >( player );

    for ( auto &var : vec3d_vars )
        var.last_value = *reinterpret_cast< vector_3d * >( player_addr + var.offset );

    for ( auto &var : float_vars )
        var.last_value = *reinterpret_cast< float * >( player_addr + var.offset );
}

void netvar_compression::post_update( c_cs_player *player ) {
    // make sure we atleast have 1 record before current command
    if ( !written_pre_vars )
        return;

    auto player_addr = reinterpret_cast< size_t >( player );

    for ( auto &var : vec3d_vars ) {
        vector_3d &value = *reinterpret_cast< vector_3d * >( player_addr + var.offset );
        value = get_new( var.last_value, value, var.tolerance );
    }

    for ( auto &var : float_vars ) {
        float &value = *reinterpret_cast< float * >( player_addr + var.offset );
        value = get_new( var.last_value, value, var.tolerance );
    }
}

float assign_range_multiplier( int bits, double range ) {
    unsigned long high_value;
    if ( bits == 32 )
        high_value = 0xFFFFFFFE;
    else
        high_value = ( ( 1 << ( unsigned long ) bits ) - 1 );

    float high_low_mult = high_value / range;
    if ( fabs( range ) <= EQUAL_EPSILON )// CloseEnough call
        high_low_mult = high_value;

    // If the precision is messing us up, then adjust it so it won't.
    if ( ( unsigned long ) ( high_low_mult * range ) > high_value ||
         ( high_low_mult * range ) > ( double ) high_value ) {
        // Squeeze it down smaller and smaller until it's going to produce an integer
        // in the valid range when given the highest value.
        float multipliers[] = { 0.9999f, 0.99f, 0.9f, 0.8f, 0.7f };
        int i;
        for ( i = 0; i < ARRAYSIZE( multipliers ); i++ ) {
            high_low_mult = ( float ) ( high_value / range ) * multipliers[ i ];
            if ( ( unsigned long ) ( high_low_mult * range ) > high_value || ( high_low_mult * range ) > ( double ) high_value ) {
            } else
                break;
        }

        if ( i == ARRAYSIZE( multipliers ) )
            // Doh! We seem to be unable to represent this range.
            return 0;
    }

    return high_low_mult;
}