#include "run_command.hpp"
#include <features/animations/animation_sync.hpp>

int last_command_time = 0;

void __fastcall hooks::run_command::hook( REGISTERS, c_base_player *player, c_user_cmd *ucmd, c_move_helper *move_helper ) {
    g_interfaces.move_helper = move_helper;

    if ( ucmd->tick_count >= std::numeric_limits< int >::max( ) )
        return;

    if ( !globals::user_cmd || !ucmd || !globals::local_player || !globals::local_player->alive( ) ) {
        last_command_time = 0;
        return original.fastcall< void >( REGISTERS_OUT, player, ucmd, move_helper );
    }

    original.fastcall< void >( REGISTERS_OUT, player, ucmd, move_helper );
}

void hooks::run_command::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.prediction, 19 ),
                                          run_command::hook );
}