#include "get_alpha_modulation.hpp"

float __fastcall hooks::get_alpha_modulation::hook( REGISTERS ) {
    const auto material = reinterpret_cast< i_material * >( ecx );

    if ( !material )
        return original.fastcall< float >( REGISTERS_OUT );

    if ( material->is_error_material( ) || !strstr( material->get_texture_group_name( ), _xs( "StaticProp textures" ) ) )
        return original.fastcall< float >( REGISTERS_OUT );

    return 1.0f - ( g_vars.visuals_other_prop_transparency.value / 100.f );
}

void hooks::get_alpha_modulation::init( ) {
    original = safetyhook::create_inline( signature::find( "materialsystem.dll", "55 8B EC 83 EC 0C 56 8B F1 8A 46 20 C0 E8 02 A8" ).sub( 48 ).get< void * >( ),
                                          get_alpha_modulation::hook );
}