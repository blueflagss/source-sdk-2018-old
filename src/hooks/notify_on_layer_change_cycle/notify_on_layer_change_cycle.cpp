#include "notify_on_layer_change_cycle.hpp"

void __fastcall hooks::notify_on_layer_change_cycle::hook( REGISTERS, const c_animation_layer *layer, const float new_cycle ) {
    const auto state = reinterpret_cast< c_csgo_player_animstate * >( ecx );

    if ( !state || !state->m_pPlayer || state->m_pPlayer != globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, layer, new_cycle );


    return;
}

void hooks::notify_on_layer_change_cycle::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 8B 45 08 85 C0 74 38 80 B9 ? ? ? ? ? 74 2F 56 8B B1 ? ? ? ? 85 F6 74 23 8D 4D 08 51 50 8B CE E8 ? ? ? ? 84 C0 74 13 83 7D 08 06 75 0D F3 0F 10 45 ? F3 0F 11 86 ? ? ? ? 5E 5D C2 08 00 CC CC CC CC CC CC CC CC CC CC 55 8B EC A1" ) ).get< void * >( ),
                                          notify_on_layer_change_cycle::hook );
}