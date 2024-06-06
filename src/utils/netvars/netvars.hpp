#pragma once
#include <globals.hpp>
#include <sdk/hash/fnv1a.hpp>
#include <sdk/interfaces/base_client.hpp>

#define NETVAR( name, type, table, prop, ... )                                                                     \
    __inline type &name( ) {                                                                                       \
        auto offset = g_netvars.get_offset( HASH_CT( table ), HASH_CT( prop ) );                                   \
        return *reinterpret_cast< type * >( reinterpret_cast< std::uintptr_t >( this ) + ( offset __VA_ARGS__ ) ); \
    }

#define OFFSET( name, type, offset )                                                               \
    __inline type &name( ) {                                                                       \
        return *reinterpret_cast< type * >( reinterpret_cast< std::uintptr_t >( this ) + offset ); \
    }

#define POFFSET( name, type, offset )                                                              \
    __inline type name( ) {                                                                        \
        return *reinterpret_cast< type * >( reinterpret_cast< std::uintptr_t >( this ) + offset ); \
    }

struct netvar_data {
    size_t offset;
    c_recv_prop *prop_ptr;
    const char* var_name;
    const char *table_name;
};

class netvars {
public:
    void init( );
    void store_table( const char *name, c_recv_table *table, std::size_t offset = 0ul );
    std::size_t get_offset( const hash32_t &table_name, const hash32_t &prop_name );

protected:
    std::unordered_map< hash32_t, std::unordered_map< hash32_t, netvar_data > > netvar_map = { };
};

inline netvars g_netvars = { };