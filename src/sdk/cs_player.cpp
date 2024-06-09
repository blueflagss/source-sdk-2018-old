#include "cs_player.hpp"

studiohdr_t *c_cs_player::get_model_ptr( ) {
    static auto get_model_ptr = signature::find( "client.dll", XOR( "E8 ? ? ? ? 83 C4 04 8B C8 E8 ? ? ? ? 83 B8" ) ).add( 0x1 ).rel32( ).get< studiohdr_t *( __thiscall * ) ( void * ) >( );

    return get_model_ptr( this );
}

int c_cs_player::lookup_pose_parameter( const char *name ) {
    auto model_ptr = get_model_ptr( );

    if ( !model_ptr )
        return -1;

    static auto lookup_pose_parameter = signature::find( "client.dll", XOR( "55 8B EC 57 8B 7D 08 85 FF 75 08" ) ).get< int( __thiscall * )( void *, void *, const char * ) >( );

    return lookup_pose_parameter( this, model_ptr, name );
}

float c_cs_player::set_pose_parameter( int index, float value ) {
    static auto addr = signature::find( "client.dll", XOR( "E8 ? ? ? ? D9 45 08 5F" ) ).add( 0x1 ).rel32( ).get< void * >( );

    __asm {
			push index
			movss xmm2, value
			mov ecx, this
			call addr
    }
}

float c_cs_player::sequence_duration( int sequence ) {
    static auto sequence_duration = signature::find( XOR( "client.dll" ), XOR( "55 8B EC 56 8B F1 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 83 C4 04 5E 5D C2 04 00 52 68 ? ? ? ? 6A 02" ) ).get< float( __thiscall * )( void *, int ) >( );

    float retval;
    sequence_duration( this, sequence );
    __asm movss retval, xmm0;

    return retval;
}

float c_cs_player::get_sequence_cycle_rate( int sequence ) {
    float t = sequence_duration( sequence );

    if ( t > 0.0f )
        return 1.0f / t;
    else
        return 1.0f / 0.1f;
}

float c_cs_player::get_pose_parameter( int index ) {
    static auto get_pose_parameter = signature::find( "client.dll", "E8 ? ? ? ? 8B 44 24 2C 83 EC 08 F3 0F 5A" ).add( 0x1 ).rel32( ).get< float( __thiscall * )( void *, int ) >( );

    return get_pose_parameter( this, index );
}

void c_cs_player::get_bone_position( int bone, vector_3d &out, vector_3d &q ) {
    static auto get_bone_position = signature::find( "client.dll", "E8 ? ? ? ? 8D 54 24 0C" ).add( 0x1 ).rel32( ).get< void( __thiscall * )( void *, int, vector_3d &, vector_3d & ) >( );

    return get_bone_position( this, bone, out, q );
}

int c_cs_player::lookup_bone( const char *name ) {
    static auto lookup_bone = signature::find( "client.dll", "55 8B EC 53 56 8B F1 57 83 ? ? ? ? ? ? 75" ).get< int( __thiscall * )( void *, const char * ) >( );

    return lookup_bone( this, name );
}

void c_cs_player::modify_eye_position( c_csgo_player_animstate *state, vector_3d *input_eye_pos, matrix_3x4 *bones ) {
    const auto e = state->m_pPlayer;

    if ( !e )
        return;

    if ( !bones )
        return;

    state->m_bSmoothHeightValid = false;

    if ( state->m_pPlayer && ( state->m_bLanding || state->m_flAnimDuckAmount != 0.f || !state->m_pPlayer->ground_entity_handle( ) ) ) {
        const auto head_bone = e->lookup_bone( XOR( "head_0" ) );

        if ( head_bone != -1 ) {
            const vector_3d head_pos( bones[ 8 ][ 0 ][ 3 ], bones[ 8 ][ 1 ][ 3 ], bones[ 8 ][ 2 ][ 3 ] );

            float v6 = head_pos.z + 1.7f;

            if ( input_eye_pos->z > v6 ) {
                float v8 = 0.0f;
                float v9 = ( float ) ( fabs( input_eye_pos->z - v6 ) - 4.0f ) * 0.16666667f;

                if ( v9 >= 0.0 )
                    v8 = fminf( v9, 1.0f );

                input_eye_pos->z = ( ( v6 - input_eye_pos->z ) * ( ( ( v8 * v8 ) * 3.0f ) - ( ( ( v8 * v8 ) * 2.0f ) * v8 ) ) ) + input_eye_pos->z;
            }
        }
    }
}

int c_cs_player::lookup_sequence( const char *label ) {
    static auto lookup_sequence = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 8B D0 89 54 24 18 83 FA FF 75 11" ) ).add( 0x1 ).rel32( ).get< int( __thiscall * )( void *, const char * ) >( );

    return lookup_sequence( this, label );
}

int c_cs_player::get_sequence_activity( int sequence ) {
    static auto get_sequence_activity = signature::find( XOR( "client.dll" ), XOR( "53 56 8B F1 8B DA 85 F6 74 55" ) ).get< int( __fastcall * )( studiohdr_t *, int ) >( );

    return get_sequence_activity( get_model_ptr( ), sequence );
}

void c_cs_player::invalidate_bone_cache( ) {
    static auto &g_model_bone_counter = *signature::find( XOR( "client.dll" ), XOR( "3B 05 ? ? ? ? 0F 84 ? ? ? ? 8B 47" ) ).add( 2 ).deref( ).get< int * >( );

    *reinterpret_cast< float * >( reinterpret_cast< uintptr_t >( this ) + 0x2914 ) = 0xFF7FFFFF;
    *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( this ) + 0x2680 ) = g_model_bone_counter - 1;
}

bool c_cs_player::can_attack( ) {
    const auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( this->weapon_handle( ) );

    if ( !weapon )
        return false;

    const auto next_primary_attack = weapon->next_primary_attack( );
    const auto time = g_interfaces.global_vars->interval_per_tick * this->tick_base( );

    if ( weapon->clip_1( ) == 0 )
        return false;

    return next_primary_attack <= time && this->next_attack( ) <= time;
}

bool c_cs_player::get_aim_matrix( vector_3d angle, matrix_3x4 *bones ) { /* inure */
    return false;
}

vector_3d c_cs_player::get_shoot_position( ) {
    auto pos = this->origin( ) + this->view_offset( );
    utils::get_method< void( __thiscall * )( void *, vector_3d * ) >( this, 163 )( this, &pos );

    if ( use_new_animstate( ) ) {
        auto state = anim_state( );

        if ( state )
            modify_eye_position( state, &pos, this->bone_cache( ) );
    }

    return pos;
}