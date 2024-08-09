#include "draw_set_color.hpp"

void __fastcall hooks::draw_set_color::hook( REGISTERS, int r, int g, int b, int a ) {
    if ( !g_vars.misc_modulate_console.value )
        return original.fastcall< void >( REGISTERS_OUT, r, g, b, a );

    if ( globals::is_console_toggled ) {
        const auto new_r = std::min< int >( 255, static_cast< int >( g_vars.misc_console_modulation_color.value.r + ( r / 10.0f ) ) );
        const auto new_g = std::min< int >( 255, static_cast< int >( g_vars.misc_console_modulation_color.value.g + ( g / 10.0f ) ) );
        const auto new_b = std::min< int >( 255, static_cast< int >( g_vars.misc_console_modulation_color.value.b + ( b / 10.0f ) ) );

        return original.fastcall< void >( REGISTERS_OUT, new_r, new_g, new_b, a );
    }

    return original.fastcall< void >( REGISTERS_OUT, r, g, b, a );
}

void hooks::draw_set_color::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.surface, 15 ),
                                          draw_set_color::hook );
}