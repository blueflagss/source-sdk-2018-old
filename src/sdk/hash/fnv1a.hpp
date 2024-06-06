#pragma once
#include <utils/utils.hpp>

using hash32_t = std::uint32_t;

namespace fnv1a
{
    constexpr std::uint32_t fnv_prime_value = 0x01000193;

    __forceinline consteval hash32_t hash_ctime( const char *input, unsigned val = 0x811c9dc5 ) noexcept {
        return input[ 0 ] == '\0' ? val : hash_ctime( input + 1, ( val ^ *input ) * fnv_prime_value );
    }

    __forceinline constexpr hash32_t hash_rtime( const char *input, unsigned val = 0x811c9dc5 ) noexcept {
        return input[ 0 ] == '\0' ? val : hash_rtime( input + 1, ( val ^ *input ) * fnv_prime_value );
    }
}// namespace fnv1a

#define HASH( x ) fnv1a::hash_rtime( x )

#define HASH_CT( x ) fnv1a::hash_ctime( x )
