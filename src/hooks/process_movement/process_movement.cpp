#include "process_movement.hpp"

void __fastcall hooks::process_movement::hook( REGISTERS, c_base_entity *player, c_move_data *data ) {
    data->game_code_moved_player = false;
    return original.fastcall< void >( REGISTERS_OUT, player, data );
}

void hooks::process_movement::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.game_movement, 1 ),
                                          process_movement::hook );
}