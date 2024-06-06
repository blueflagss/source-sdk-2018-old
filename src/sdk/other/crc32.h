#pragma once
// used: [stl] uint32_t
#include <cstdint>

using CRC32_t = std::uint32_t;

namespace crc_32
{
    void init( CRC32_t *pulCRC );
    void final( CRC32_t *pulCRC );
    CRC32_t get_table_entry( const unsigned int nSlot );
    void process_buffer( CRC32_t *pulCRC, const void *pBuffer, int nBufferSize );

    inline CRC32_t process_single_buffer( const void *pBuffer, const int nBufferSize ) {
        CRC32_t uCRC;
        init( &uCRC );
        process_buffer( &uCRC, pBuffer, nBufferSize );
        final( &uCRC );
        return uCRC;
    }
}// namespace crc_32