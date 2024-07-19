#include "level_shutdown.hpp"
#include <features/shot_manager/shot_manager.hpp>
#include <features/sound_handler/sound_handler.hpp>

void __fastcall hooks::level_shutdown::hook( REGISTERS ) {
    globals::local_weapon_data = nullptr;
    globals::local_weapon = nullptr;

    g_shot_manager.reset_data( );
    g_sound_handler.reset_data( );

    globals::local_player = nullptr;

    return original.fastcall< void >( REGISTERS_OUT );
}

void hooks::level_shutdown::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.client_mode, 26 ),
                                          level_shutdown::hook );
}