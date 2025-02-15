#include "level_init_pre_entity.hpp"
#include <features\ragebot\ragebot.hpp>

void __fastcall hooks::level_init_pre_entity::hook( REGISTERS, const char *map ) {
    globals::tick_rate = 1.0f / g_interfaces.global_vars->interval_per_tick;

    // set rates when joining a server.
    globals::cvars::cl_updaterate->set_float( globals::tick_rate );
    globals::cvars::cl_cmdrate->set_float( globals::tick_rate );

    g_ragebot.reset( );

    return original.fastcall< void >( REGISTERS_OUT, map );
}

void hooks::level_init_pre_entity::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client, 5 ),
                                          level_init_pre_entity::hook );
}