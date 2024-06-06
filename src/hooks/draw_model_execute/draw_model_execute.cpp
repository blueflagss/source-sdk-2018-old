#include "draw_model_execute.hpp"
#include <features/visuals/chams.hpp>

void __fastcall hooks::draw_model_execute::hook( REGISTERS, void *render_context, const draw_model_state_t &state, const model_render_info_t &info, matrix_3x4 *bone_to_world ) {
    const auto original = reinterpret_cast< decltype( &hooks::draw_model_execute::hook ) >( hooks::draw_model_execute::original.trampoline( ).address( ) );

    auto entity = g_interfaces.entity_list->get_client_entity< c_cs_player * >( info.entity_index );

    if ( !entity )
        return original( REGISTERS_OUT, render_context, state, info, bone_to_world );

    if ( HASH( entity->get_client_class( )->network_name ) != HASH_CT( "CCSPlayer" ) )
        return original( REGISTERS_OUT, render_context, state, info, bone_to_world );

    if ( !g_chams.draw_model_execute( reinterpret_cast< i_model_render * >( ecx ), edx, render_context, state, info, bone_to_world, entity ) )
        return original( REGISTERS_OUT, render_context, state, info, bone_to_world );
}

void hooks::draw_model_execute::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.model_render, 21 ),
                                          draw_model_execute::hook );
}