#include "prediction_copy.hpp"

c_prediction_copy::c_prediction_copy( int type, char *dest, bool dest_packed, const char *src, bool src_packed, optype_t optype, FN_FIELD_COMPARE func /*= NULL*/ ) {
    optype = optype;
    num_type = type;
    dest = dest;
    src = src;
    destoffsetindex = dest_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
    srcoffsetindex = src_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
    error_count = 0;
    entindex = -1;
    watch_field = 0;
    field_compare_fn = func;
}

int c_prediction_copy::transfer_data( const char *operation, int idx, datamap_t *dmap ) {
    static auto transfer_data = signature::find( XOR( "client.dll" ), XOR( "55 8B EC 8B 45 10 53 56 8B F1 57 8B 7D 0C 89 7E 1C 85" ) ).get< int( __thiscall * )( c_prediction_copy *, const char *, int, datamap_t * ) >( );

    return transfer_data( this, operation, idx, dmap );
}

typedescription_t *c_prediction_copy::find_flat_field_by_name( const char *field_name, const datamap_t *datamap ) {
    static auto find_flat_field_by_name = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 8B CF 8B 40 34 03 45 08 F3 0F 7E 00" ) ).add( 0x1 ).rel32( ).get< typedescription_t *( __fastcall * ) ( const char *, const datamap_t * ) >( );

    return find_flat_field_by_name( field_name, datamap );
}
//
//bool c_prediction_copy::prepare_datamap( datamap_t *dmap ) {
//
//    bool performed_prepare = false;
//
//    if ( dmap && !dmap->optimized_datamap ) {
//        performed_prepare = true;
//        build_flattened_chains( dmap );
//        dmap = dmap->base_map;
//    }
//
//    return performed_prepare;
//}