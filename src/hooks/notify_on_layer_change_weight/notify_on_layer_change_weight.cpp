#include "notify_on_layer_change_weight.hpp"

void __fastcall hooks::notify_on_layer_change_weight::hook( REGISTERS, const c_animation_layer *layer, const float new_weight ) {
    const auto state = reinterpret_cast< c_csgo_player_animstate * >( ecx );

    if ( !state || !state->m_pPlayer || state->m_pPlayer != globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT, layer, new_weight );

    return;
}

void hooks::notify_on_layer_change_weight::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 8B 45 08 85 C0 74 1C" ) ).get< void * >( ),
                                          notify_on_layer_change_weight::hook );
}