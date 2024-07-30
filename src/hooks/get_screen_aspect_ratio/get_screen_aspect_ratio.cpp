#include "get_screen_aspect_ratio.hpp"

float __fastcall hooks::get_screen_aspect_ratio::hook( REGISTERS, int viewport_width, int viewport_height ) {
    const auto ratio = original.fastcall< float >( REGISTERS_OUT, viewport_width, viewport_height );

    if ( g_vars.visuals_other_aspect_ratio.value > 0.0f )
        return g_vars.visuals_other_aspect_ratio.value / 50.0f;

    return ratio;
}

void hooks::get_screen_aspect_ratio::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.engine_client, 101 ),
                                          get_screen_aspect_ratio::hook );
}