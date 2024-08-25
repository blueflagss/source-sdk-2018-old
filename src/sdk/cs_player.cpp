#include "cs_player.hpp"
#include <features/animations/animation_sync.hpp>

studiohdr_t *c_cs_player::get_model_ptr( ) {
    static auto get_model_ptr = signature::find( "client.dll", _xs( "E8 ? ? ? ? 83 C4 04 8B C8 E8 ? ? ? ? 83 B8" ) ).add( 0x1 ).rel32( ).get< studiohdr_t *( __thiscall * ) ( void * ) >( );

    return get_model_ptr( this );
}

float c_cs_player::get_first_sequence_anim_tag( int sequence, int desired_tag, float start, float end ) {
    static auto get_first_sequence_anim_tag = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? F3 0F 11 86 ? ? ? ? 0F 57 DB E9" ) ).add( 0x1 ).rel32( ).get< float( __thiscall * )( void *, int, int, float, float ) >( );

    const auto model_ptr = get_model_ptr( );
    auto ret = 0.0f;

    __asm {
		push model_ptr
		push desired_tag
		push sequence
		mov ecx, this
		call get_first_sequence_anim_tag
		movss ret, xmm0
    }

    return ret;
}

void c_cs_player::get_sequence_linear_motion( void *hdr, int seq, const float poses[], vector_3d *vec ) {
    static auto addr = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 EC 0C 56 8B F1 57 8B FA 85 F6 75 14" ) ).get< void( __thiscall * )( void *, void *, int, const float[], vector_3d * ) >( );

    __asm {
			mov edx, seq
			mov ecx, hdr
			push vec
			push poses
			call addr
			add esp, 8
    }
}

float c_cs_player::get_sequence_move_dist( void *hdr, int seq ) {
    vector_3d ret{ };

    get_sequence_linear_motion( hdr, seq, this->pose_parameters( ).data( ), &ret );

    return glm::length( ret );
}

int c_cs_player::lookup_pose_parameter( const char *name ) {
    auto model_ptr = get_model_ptr( );

    if ( !model_ptr )
        return -1;

    static auto lookup_pose_parameter = signature::find( "client.dll", _xs( "55 8B EC 57 8B 7D 08 85 FF 75 08" ) ).get< int( __thiscall * )( void *, void *, const char * ) >( );

    return lookup_pose_parameter( this, model_ptr, name );
}

float c_cs_player::set_pose_parameter( int index, float value ) {
    static auto addr = signature::find( "client.dll", _xs( "E8 ? ? ? ? D9 45 08 5F" ) ).add( 0x1 ).rel32( ).get< void * >( );

    __asm {
			push index
			movss xmm2, value
			mov ecx, this
			call addr
    }
}

float c_cs_player::sequence_duration( int sequence ) {
    static auto sequence_duration = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 56 8B F1 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 83 C4 04 5E 5D C2 04 00 52 68 ? ? ? ? 6A 02" ) ).get< float( __thiscall * )( void *, int ) >( );

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

    if ( state->m_pPlayer && ( state->m_bLanding || state->m_flAnimDuckAmount != 0.f || !g_interfaces.entity_list->get_client_entity_from_handle< c_base_entity * >( state->m_pPlayer->ground_entity_handle( ) ) ) ) {
        const auto head_bone = e->lookup_bone( _xs( "head_0" ) );

        if ( head_bone != -1 ) {
            const vector_3d head_pos( bones[ 8 ][ 0 ][ 3 ], bones[ 8 ][ 1 ][ 3 ], bones[ 8 ][ 2 ][ 3 ] );

            const auto bone_z = head_pos.z + 1.7f;
            if ( input_eye_pos->z > bone_z ) {
                const auto view_modifier = std::clamp( ( fabsf( input_eye_pos->z - bone_z ) - 4.f ) * .16666667f, 0.f, 1.f );
                const auto view_modifier_sqr = view_modifier * view_modifier;

                input_eye_pos->z += ( bone_z - input_eye_pos->z ) * ( 3.f * view_modifier_sqr - 2.f * view_modifier_sqr * view_modifier );
            }
        }
    }
}

int &c_cs_player::anim_lod_flags( ) {
    static auto anim_lod_flags_offset = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? F3 0F 10 45 ? 51" ) ).add( 0x1 ).rel32( ).add( 97 ).deref( ).get< uintptr_t >( );

    return *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( this ) + anim_lod_flags_offset );
}

int c_cs_player::lookup_sequence( const char *label ) {
    static auto lookup_sequence = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 8B D0 89 54 24 18 83 FA FF 75 11" ) ).add( 0x1 ).rel32( ).get< int( __thiscall * )( void *, const char * ) >( );

    return lookup_sequence( this, label );
}

int c_cs_player::get_sequence_activity( int sequence ) {
    static auto get_sequence_activity = signature::find( _xs( "client.dll" ), _xs( "53 56 8B F1 8B DA 85 F6 74 55" ) ).get< int( __fastcall * )( studiohdr_t *, int ) >( );

    return get_sequence_activity( get_model_ptr( ), sequence );
}

void c_cs_player::invalidate_bone_cache( ) {
    static auto &g_model_bone_counter = *signature::find( _xs( "client.dll" ), _xs( "3B 05 ? ? ? ? 0F 84 ? ? ? ? 8B 47" ) ).add( 2 ).deref( ).get< int * >( );

    *reinterpret_cast< float * >( reinterpret_cast< uintptr_t >( this ) + 0x2914 ) = 0xFF7FFFFF;
    *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( this ) + 0x2680 ) = g_model_bone_counter - 1;
}

bool c_cs_player::can_attack( ) {
    const auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_cs_weapon_base * >( this->weapon_handle( ) );

    if ( !weapon )
        return false;

    const auto next_primary_attack = weapon->next_primary_attack( );
    const auto time = game::ticks_to_time( globals::local_player->tick_base( ) );
    const auto weapon_definition_idx = weapon->item_definition_index( );

    if ( weapon->clip_1( ) == 0 )
        return false;

    if ( ( weapon_definition_idx == weapons::glock || weapon_definition_idx == weapons::famas ) && weapon->burst_shots_remaining( ) > 0 ) {
        if ( time >= weapon->next_burst_shot( ) )
            return true;
    }

    //if ( weapon_definition_idx == weapons::revolver ) {
    //    int act = weapon->activity( );

    //    if ( !globals::fired_revolver ) {
    //        if ( ( act == 185 || act == 193 ) && revolver_cock == 0 )
    //            return g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack( );

    //        return false;
    //    }
    //}

    return next_primary_attack <= time && this->next_attack( ) <= time;
}

bool c_cs_player::get_aim_matrix( vector_3d angle, matrix_3x4 *bones ) { /* inure */
    return false;
}

void c_cs_player::update_collision_bounds( ) {
    return utils::get_method< void( __thiscall * )( void * ) >( this, 329 )( this );
}

void c_cs_player::standard_blending_rules( c_studio_hdr *hdr, vector_3d *pos, vector_4d *q, float time, int mask ) {
    return utils::get_method< void( __thiscall * )( void *, c_studio_hdr *, vector_3d *, vector_4d *, float, int ) >( this, 200 )( this, hdr, pos, q, time, mask );
}

void c_cs_player::build_transformations( c_studio_hdr *hdr, vector_3d *pos, vector_4d *q, const matrix_3x4_aligned &mat, int mask, uint8_t *computed ) {
    return utils::get_method< void( __thiscall * )( void *, c_studio_hdr *, vector_3d *, vector_4d *, const matrix_3x4_aligned &, int, uint8_t * ) >( this, 184 )( this, hdr, pos, q, mat, mask, computed );
}

void c_cs_player::update_ik_locks( float time ) {
    return utils::get_method< void( __thiscall * )( void *, float ) >( this, 186 )( this, time );
}

std::deque< c_base_attribute_item * > c_cs_player::weapons( ) {
    static auto offset = g_netvars.get_offset( HASH_CT( "DT_BaseCombatCharacter" ), HASH_CT( "m_hMyWeapons" ) );

    std::deque< c_base_attribute_item * > ret{ };

    const auto my_weapons = reinterpret_cast< uint32_t * >( reinterpret_cast< uintptr_t >( this ) + offset );

    for ( auto i = 0; my_weapons[ i ] != 0xFFFFFFFF; i++ ) {
        const auto weapon = g_interfaces.entity_list->get_client_entity_from_handle< c_base_attribute_item * >( my_weapons[ i ] );

        if ( !weapon )
            continue;

        ret.push_back( weapon );
    }

    return ret;
}

vector_3d c_cs_player::get_shoot_position( ) {
    auto pos = origin( );

    if ( this == globals::local_player ) {
        pos += view_offset( );
        utils::get_method< void( __thiscall * )( c_cs_player *, vector_3d & ) >( this, 163 )( this, pos );

        auto state = anim_state( );

        if ( state )
            modify_eye_position( state, &pos, this->bone_cache( ) );
    }

    else {
        pos += g_interfaces.game_movement->get_player_view_offset( false );

        if ( flags( ) & on_ground )
            pos -= ( g_interfaces.game_movement->get_player_view_offset( false ) - g_interfaces.game_movement->get_player_view_offset( true ) ) * duck_amount( );
    }

    return pos;
}