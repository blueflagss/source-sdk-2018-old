#pragma once
#include <globals.hpp>
#include <sdk/math/matrix3x4.hpp>
#include "signature/signature.hpp"

#define REGISTERS void *ecx, void *edx
#define REGISTERS_OUT ecx, edx

class c_base_entity;

namespace utils
{
    template< typename T >
    inline T get_method( void *table_base, std::uintptr_t index ) {
        return ( T ) ( ( *reinterpret_cast< std::uintptr_t ** >( table_base ) )[ index ] );
    }

    bool is_point_visible( vector_3d pos_to_trace, c_base_entity *entity, int *hit_group );
    std::string convert_utf8( wchar_t *text );
    std::vector< std::string > split_str( std::string str, const char separator );
}// namespace utils