#include "do_extra_bone_processing.hpp"

void __fastcall hooks::do_extra_bone_processing::hook( REGISTERS, c_studio_hdr *hdr, vector_3d pos[ ], quat_aligned q[ ], float current_time, int bone_mask ) {
    auto player = reinterpret_cast< c_cs_player * >( ecx );

    if ( !player )
        return original.fastcall< void >( REGISTERS_OUT, hdr, pos, q, current_time, bone_mask );
    
    auto state = player->anim_state( );

    if ( !state )
        return original.fastcall< void >( REGISTERS_OUT, hdr, pos, q, current_time, bone_mask );
    
    const auto backup_on_ground = state->m_bOnGround;

    state->m_bOnGround = true;
    original.fastcall< void >( REGISTERS_OUT, hdr, pos, q, current_time, bone_mask );
    state->m_bOnGround = backup_on_ground;
}

void hooks::do_extra_bone_processing::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "57 8B F9 8B 07 8B ? ? ? ? ? FF D0 84 C0 75 02" ) ).get< void * >( ),
                                          do_extra_bone_processing::hook );
}