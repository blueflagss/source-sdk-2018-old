#include "run_command.hpp"

void __fastcall hooks::run_command::hook( REGISTERS, c_base_player *player, c_user_cmd *ucmd, c_move_helper *move_helper ) {
    g_interfaces.move_helper = move_helper;

    return original.fastcall< void >( REGISTERS_OUT, player, ucmd, move_helper );
}

void hooks::run_command::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.prediction, 19 ),
                                          run_command::hook );
}