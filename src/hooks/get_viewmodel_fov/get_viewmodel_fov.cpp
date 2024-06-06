#include "get_viewmodel_fov.hpp"

float __fastcall hooks::get_viewmodel_fov::hook( REGISTERS ) {
    return g_vars.visuals_other_viewmodel_fov.value > 0 ? g_vars.visuals_other_viewmodel_fov.value : original.fastcall< float >( REGISTERS_OUT );
}

void hooks::get_viewmodel_fov::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client_mode, 35 ),
                                          get_viewmodel_fov::hook );
}