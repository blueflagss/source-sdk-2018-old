#pragma once
#include <globals.hpp>

struct address {
    std::uint8_t *m_addr;

public:
    inline address( ) {
        this->m_addr = nullptr;
    }

    inline address( std::uint8_t *addr ) {
        this->m_addr = addr;
    }

    template< typename T >
    inline T get( ) {
        return T( m_addr );
    }

    template< typename T = address >
    inline address deref( ) {
        return address( *reinterpret_cast< std::uint8_t ** >( this->m_addr ) );
    }

    inline address rel32( ) {
        return address( this->m_addr + *reinterpret_cast< std::int32_t * >( this->m_addr ) + sizeof( std::int32_t ) );
    }

    inline address sub( std::uintptr_t bytes ) {
        return address( m_addr - bytes );
    }

    inline address add( std::uintptr_t bytes ) {
        return address( m_addr + bytes );
    }
};

namespace signature
{
    address find( const char *module_name, const char *pattern_bytes );
}// namespace signature