#pragma once
#include <globals.hpp>

class animation_context {
public:
    animation_context( ) {
    }

    animation_context( float value, float last_value, hash32_t item_hash ) {
        this->value = value;
        this->last_value = value;
        this->item_hash = item_hash;
    }

    float value;
    float last_value;
    hash32_t item_hash;
};

namespace animations {
    animation_context get( const hash32_t &hash, float default_value = 0.f );
    void clear_map( );
    void set_values( );
    void lerp_to( const hash32_t &hash, float destination, float additive = 0.5f, float min_max_clamp = 1000.f );
    void set_to( const hash32_t &hash, float wanted_value );
}// namespace animations