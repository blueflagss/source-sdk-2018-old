#include "lower_body_yaw_target_proxy.hpp"
#include <features/resolver/resolver.hpp>

void __cdecl hooks::lower_body_yaw_target_proxy::hook( const c_recv_proxy_data *data, void *player, void *a3 ) {
    static auto recvtable_decode = signature::find( _xs( "engine.dll" ), _xs( "EB 0D FF 77 10" ) ).get< void * >( );

    if ( _ReturnAddress( ) != recvtable_decode ) {
        const auto entity = reinterpret_cast< c_cs_player * >( player );

        g_resolver.on_proxy_update( entity, data->value.f );
    }

    if ( original )
        original( data, player, a3 );
}

void hooks::lower_body_yaw_target_proxy::init( ) {
    g_netvars.set_proxy( HASH_CT( "DT_CSPlayer" ), HASH_CT( "m_flLowerBodyYawTarget" ), lower_body_yaw_target_proxy::hook, original );
}