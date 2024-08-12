#include "sequence_proxy.hpp"
#include <features/skin_changer/skin_changer.hpp>

void __cdecl hooks::sequence_proxy::hook( c_recv_proxy_data *data, void *entity, void *a3 ) {
    g_skin_changer.do_sequence_remapping( data, reinterpret_cast< c_base_view_model * >( entity ) );

    return original( data, entity, a3 );
}

void hooks::sequence_proxy::init( ) {
    g_netvars.set_proxy( HASH_CT( "DT_BaseViewModel" ), HASH_CT( "m_nSequence" ), sequence_proxy::hook, original );
}