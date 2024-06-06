#include "animations.hpp"
#include <core/variables.hpp>

std::unordered_map< hash32_t, animation_context > anim_map;

animation_context animations::get( const hash32_t &hash, float default_value ) {
    auto query = anim_map.find( hash );

    if ( query == anim_map.end( ) ) {
        anim_map.insert( std::pair< hash32_t, animation_context >( hash, animation_context( default_value, 0.f, hash ) ) );
        query = anim_map.find( hash );
    }

    return query->second;
}

void animations::clear_map( ) {
    anim_map.clear( );
}

void animations::lerp_to( const hash32_t &hash, float destination, float additive, float min_max_clamp ) {
    const auto map = get( hash );
    auto value = anim_map.find( map.item_hash );

    if ( value->second.value != std::clamp< float >( destination, -min_max_clamp, min_max_clamp ) )
        value->second.value = std::clamp< float >( value->second.value + ( destination - value->second.value ) * ( ( 1.f / additive ) * ImGui::GetIO( ).DeltaTime ), -min_max_clamp, min_max_clamp );
}

void animations::set_to( const hash32_t &hash, float wanted_value ) {
    const auto map = get( hash );
    auto value = anim_map.find( map.item_hash );
    value->second.value = wanted_value;
}