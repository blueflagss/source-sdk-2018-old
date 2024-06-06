#include "list_leaves_in_box.hpp"
#include <features/visuals/chams.hpp>

struct renderable_info_t {
    i_client_renderable *renderable;
    void *alpha_property;
    int enum_count;
    int render_frame;
    unsigned short first_shadow;
    unsigned short leaf_list;
    short area;
    uint16_t flags;
    uint16_t flags2;
    vector_3d bloated_abs_mins;
    vector_3d bloated_abs_maxs;
    vector_3d abs_mins;
    vector_3d abs_maxs;
    PAD( 0x4 );
};

#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )

int __fastcall hooks::list_leaves_in_box::hook( REGISTERS, const vector_3d &mins, const vector_3d &maxs, unsigned short *list, int list_max ) {
    const auto original = reinterpret_cast< decltype( &hooks::list_leaves_in_box::hook ) >( hooks::list_leaves_in_box::original.trampoline( ).address( ) );

    if ( !g_vars.visuals_other_disable_model_occlusion.value )
        return original( REGISTERS_OUT, mins, maxs, list, list_max );

    static auto insert_into_tree = signature::find( "client.dll", XOR( "8B 7D 08 8B 74 24 10 89" ) ).get< void * >( );

    if ( _ReturnAddress( ) != insert_into_tree )
        return original( REGISTERS_OUT, mins, maxs, list, list_max );

    const auto info = *reinterpret_cast< renderable_info_t ** >( reinterpret_cast< uintptr_t >( _AddressOfReturnAddress( ) ) + 0x14 );

    if ( !info || !info->renderable )
        return original( REGISTERS_OUT, mins, maxs, list, list_max );

    auto entity = reinterpret_cast< c_cs_player * >( info->renderable->get_client_unknown( )->get_base_entity( ) );

    if ( !entity || HASH( entity->get_client_class( )->network_name ) != HASH_CT( "CCSPlayer" ) )
        return original( REGISTERS_OUT, mins, maxs, list, list_max );

    info->flags &= ~0x100;
    info->flags2 |= 0xC0;

    auto map_min = vector_3d( MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT );
    auto map_max = vector_3d( MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT );

    return original( REGISTERS_OUT, map_min, map_max, list, list_max );
}

void hooks::list_leaves_in_box::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.engine_client->get_bsp_tree_query( ), 6 ),
                                          list_leaves_in_box::hook );
}