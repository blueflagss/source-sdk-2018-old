#include "standard_blending_rules.hpp"

void __fastcall hooks::standard_blending_rules::hook( REGISTERS, c_studio_hdr *hdr, vector_3d pos[], quat_aligned q[], float current_time, int bone_mask ) {
    auto player = reinterpret_cast< c_cs_player * >( ecx );

    if ( !player || player != globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, hdr, pos, q, current_time, bone_mask );

    player->effects( ) |= effects::nointerp;
    original.fastcall< void >( REGISTERS_OUT, hdr, pos, q, current_time, bone_mask );
    player->effects( ) &= ~effects::nointerp;
}

void hooks::standard_blending_rules::init( ) {
    /*original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6" ) ).get< void * >( ),
                                          standard_blending_rules::hook );*/
}