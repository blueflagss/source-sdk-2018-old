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
