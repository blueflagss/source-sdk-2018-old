#include "datamaps.hpp"

std::size_t datamaps::get_offset( datamap_t *datamap, const hash32_t &var_hash ) {
    while ( datamap ) {
        for ( int i = 0; i < datamap->data_num_fields; i++ ) {
            if ( !datamap->data_desc[ i ].field_name )
                continue;

            if ( HASH( datamap->data_desc[ i ].field_name ) == var_hash )
                return datamap->data_desc[ i ].field_offset;

            if ( datamap->data_desc[ i ].field_type == FIELD_EMBEDDED && datamap->data_desc[ i ].td )
                if ( const std::size_t offset = get_offset( datamap->data_desc[ i ].td, var_hash ); offset != 0 )
                    return offset;
        }

        datamap = datamap->base_map;
    }

    return 0x0;
}