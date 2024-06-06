#include "netvars.hpp"

void netvars::init( ) {
    auto client_class = g_interfaces.client->get_all_classes( );

    if ( !client_class )
        return;

    int node_count = 0;

    while ( client_class && client_class->next ) {
        ++node_count;

        store_table( client_class->recv_table->net_table_name, client_class->recv_table );

        // Next.
        client_class = client_class->next;
    }

    spdlog::info( "netvars: populated {} nodes", node_count );

#if 1
#ifdef _DEBUG
    //spdlog::set_level( spdlog::level::debug );

    std::ofstream file( "netvars.txt" );

    for ( auto &var : this->netvar_map ) {
        for ( auto &vars : var.second ) {
            //spdlog::info( "{}->{}: 0x{:X}", vars.second.table_name, vars.second.var_name, vars.second.offset );

            file << fmt::format( "{}->{}: 0x{:X}", vars.second.table_name, vars.second.var_name, vars.second.offset ) << std::endl;
        }
    }

    file.close( );
#endif
#endif
}

void netvars::store_table( const char *name, c_recv_table *table, std::size_t offset ) {
    auto table_hash = HASH( name );

    for ( auto n = 0; n < table->num_props; ++n ) {
        auto prop = &table->props[ n ];
        auto child = prop->data_table;

        if ( child && child->num_props > 0 )
            store_table( name, prop->data_table, prop->offset + offset );

        auto var_hash = HASH( prop->var_name );

        if ( !this->netvar_map[ table_hash ][ var_hash ].offset ) {
            this->netvar_map[ table_hash ][ var_hash ].prop_ptr = prop;
            this->netvar_map[ table_hash ][ var_hash ].offset = static_cast< std::size_t >( prop->offset + offset );
            this->netvar_map[ table_hash ][ var_hash ].table_name = name;
            this->netvar_map[ table_hash ][ var_hash ].var_name = prop->var_name;
        }
    }
}

std::size_t netvars::get_offset( const hash32_t &table_name, const hash32_t &prop_name ) {
    return this->netvar_map[ table_name ][ prop_name ].offset;
}