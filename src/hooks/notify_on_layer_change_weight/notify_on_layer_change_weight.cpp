#include "notify_on_layer_change_weight.hpp"

void __fastcall hooks::notify_on_layer_change_weight::hook( REGISTERS, const c_animation_layer *layer, const float new_weight ) {
    return;
}

void hooks::notify_on_layer_change_weight::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 8B 45 08 85 C0 74 1C" ) ).get< void * >( ),
                                          notify_on_layer_change_weight::hook );
}