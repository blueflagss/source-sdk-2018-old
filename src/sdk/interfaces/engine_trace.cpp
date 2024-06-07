#include "engine_trace.hpp"
#include <core/interfaces.hpp>

bool c_game_trace::did_hit_world( ) const {
    return entity == g_interfaces.entity_list->get_client_entity< void * >( 0 );
}

bool c_trace_filter_hitscan::should_hit_entity( void *entity_handle, int contents_mask ) {
    auto entity = reinterpret_cast< c_base_entity * >( entity_handle );

    return !entity || !entity->is_player( ) ||
           ( entity->index( ) != player->index( ) && ( entity->team() == player->team() ) );
}

trace_type c_trace_filter_hitscan::get_trace_type( ) const {
    return trace_type::everything;
}
