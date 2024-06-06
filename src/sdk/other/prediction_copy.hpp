#pragma once
#include <globals.hpp>

typedef void ( *FN_FIELD_COMPARE )( const char *classname, const char *fieldname, const char *fieldtype, bool networked, bool noterrorchecked, bool differs, bool withintolerance, const char *value );

typedef enum {
    differs = 0,
    identical,
    with_intolerance,
} difftype_t;

typedef enum {
    transferdata_copyonly = 0,       // data copying only (uses runs)
    transferdata_errorcheck_nospew,  // checks for errors, returns after first error found
    transferdata_errorcheck_spew,    // checks for errors, reports all errors to console
    transferdata_errorcheck_describe,// used by hud_pdump, dumps values, etc, for all fields
} optype_t;

class c_prediction_copy {
public:
    c_prediction_copy( int type, char *dest, bool dest_packed, const char *src, bool src_packed, optype_t optype, FN_FIELD_COMPARE func = NULL );

    optype_t optype;
    int num_type;
    char *dest;
    const char *src;
    int destoffsetindex;
    int srcoffsetindex;
    int error_count;
    int entindex;
    FN_FIELD_COMPARE field_compare_fn;
    const typedescription_t *watch_field;
    char const *operation;
    std::uintptr_t field_stack;

    int transfer_data( const char *operation, int idx, datamap_t *dmap );
    typedescription_t *find_flat_field_by_name( const char *field_name, const datamap_t *datamap );
    static bool prepare_datamap( datamap_t *dmap );
};