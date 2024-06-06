#include "is_using_static_props_debug_modes.hpp"
#include <features/visuals/chams.hpp>

bool __cdecl hooks::is_using_static_props_debug_modes::hook( ) {   
    return g_vars.visuals_other_modulate_world.value || ( g_vars.visuals_other_prop_transparency.value > 0 && g_vars.visuals_other_transparent_props.value );
}

void hooks::is_using_static_props_debug_modes::init( ) {
    original = safetyhook::create_inline( signature::find( "engine.dll", "E8 ? ? ? ? 84 C0 8B 45 08" ).add( 0x1 ).rel32( ).get< void * >( ),
                                          is_using_static_props_debug_modes::hook );
}