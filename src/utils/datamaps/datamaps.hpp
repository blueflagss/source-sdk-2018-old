#pragma once
#include <globals.hpp>
#include <sdk/datamap.hpp>
#include <sdk/hash/fnv1a.hpp>
#include <sdk/interfaces/base_client.hpp>

#define DATAMAP( name, type, datamap, prop, ... )                                                                  \
    type &name( ) {                                                                                                \
        static auto offset = g_datamaps.get_offset( datamap, HASH_CT( prop ) );                                    \
        return *reinterpret_cast< type * >( reinterpret_cast< std::uintptr_t >( this ) + ( offset __VA_ARGS__ ) ); \
    }

class datamaps {
public:
    std::size_t get_offset( datamap_t *datamap, const hash32_t &var_hash );
};

inline datamaps g_datamaps = { };