#include "physics_simulate.hpp"
#include <features/engine_prediction/engine_prediction.hpp>

void __fastcall hooks::physics_simulate::hook( REGISTERS ) {
    auto player = reinterpret_cast< c_cs_player * >( ecx );

    if ( !player || ( ( *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( player ) + 0x2A8 ) == g_interfaces.global_vars->tick_count ) || !*reinterpret_cast< bool * >( reinterpret_cast< uintptr_t >( player ) + 0x34D0 ) ) )
        return original.fastcall< void >( REGISTERS_OUT );

    if ( player == globals::local_player )
        *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( globals::local_player ) + 0x3238 ) = 0;

    original.fastcall< void >( REGISTERS_OUT );

    if ( player == globals::local_player && player->viewmodel_handle( ) != 0xFFFFFFF ) {
        const auto viewmodel = g_interfaces.entity_list->get_client_entity_from_handle< c_view_model * >( player->viewmodel_handle( ) );

        if ( viewmodel ) {
            g_prediction.weapon_cycle = viewmodel->cycle( );
            g_prediction.weapon_sequence = viewmodel->sequence( );
            g_prediction.weapon_animtime = viewmodel->animtime( );
        }
    }
}

void hooks::physics_simulate::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 21" ) ).get< void * >( ),
                                          physics_simulate::hook );
}