#include "signature.hpp"

address signature::find( const char *module_name, const char *pattern ) {
    static auto pattern_to_byte = []( const char *pattern ) {
        auto bytes = std::vector< int >{ };
        auto pattern_start = const_cast< char * >( pattern );
        auto pattern_end = const_cast< char * >( pattern ) + strlen( pattern );

        for ( auto current = pattern_start; current < pattern_end; ++current ) {
            if ( *current == '?' ) {
                ++current;

                if ( *current == '?' )
                    ++current;

                bytes.push_back( -1 );
            } else
                bytes.push_back( strtoul( current, &current, 16 ) );
        }

        return bytes;
    };

    auto handle = GetModuleHandleA( module_name );
    auto dos_headers = reinterpret_cast< PIMAGE_DOS_HEADER >( handle );
    auto headers = reinterpret_cast< PIMAGE_NT_HEADERS >( reinterpret_cast< std::uint8_t * >( handle ) + dos_headers->e_lfanew );

    auto image_size = headers->OptionalHeader.SizeOfImage;
    auto pattern_bytes = pattern_to_byte( pattern );
    auto scan_bytes = reinterpret_cast< std::uint8_t * >( handle );

    auto s = pattern_bytes.size( );
    auto d = pattern_bytes.data( );

    for ( auto i = 0ul; i < image_size - s; ++i ) {
        bool found = true;

        for ( auto j = 0ul; j < s; ++j ) {
            if ( scan_bytes[ i + j ] != d[ j ] && d[ j ] != -1 ) {
                found = false;
                break;
            }
        }

        if ( found )
            return address( &scan_bytes[ i ] );
    }

    spdlog::set_level( spdlog::level::level_enum::critical );
    spdlog::error( "Couldn't find pattern \"{}\"", pattern );

    return address( 0 );
}