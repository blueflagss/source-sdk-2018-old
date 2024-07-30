#pragma once

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <safetyhook/safetyhook.hpp>
#include <shared_mutex>
#include <shlobj_core.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <windows.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <imgui/imgui.h>
#include <imgui/imgui_freetype.h>
#include <imgui/imgui_impl_dx9.h>
#include <imgui/imgui_impl_win32.h>
#include <stb/stb.hpp>
#include <string>
#include <xorstr.hpp>

union color {
    constexpr color( ) : col( 0 ) { a = 255; }

    // should probably implement a warning for float conversions here?
    constexpr color( float r, float g, float b, float a = 255 ) : r( r ), g( g ), b( b ), a( a ) {}

    // 0xRRGGBBAA format
    constexpr color( uint32_t col ) : col( col ) {}

    // individual r, g, b, a arguments (with the same type)
    template< typename T >
        requires std::is_integral_v< T >// can't pass floats to this
    constexpr color( const T &r, const T &g, const T &b, const T &a = 255 ) : r( r ), g( g ), b( b ), a( a ) {}

    // ugly float conversion
    template< typename T >
    constexpr color( color col, const T &a ) : r( col.r ), g( col.g ), b( col.b ), a( uint8_t( a ) ) {}

    bool operator !=( color col ) {
        return col.r != this->r || col.g != this->g || col.b != this->b || col.a != this->a;
    }

    color lerp( const color &to, const float fraction ) {
        // std::lerp has bound checks we don't need here, this is faster
        return color( ( to.r - r ) * fraction + r,
                      ( to.g - g ) * fraction + g,
                      ( to.b - b ) * fraction + b,
                      ( to.a - a ) * fraction + a );
    }

    uint32_t col;

    struct {// these are backwards due to windows being little endian
        uint8_t a;
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };

    constexpr static color white( ) {
        return { 255, 255, 255 };
    }

    constexpr static color black( ) {
        return { 0, 0, 0 };
    }
};

class hsv {
    constexpr static uint32_t HSV_HUE_SEXTANT = 256;
    constexpr static uint32_t HSV_HUE_STEPS = 6 * HSV_HUE_SEXTANT;
    constexpr static uint16_t HSV_HUE_MAX = HSV_HUE_STEPS - 1;

    static uint8_t min_col( color col ) {
        if ( col.g <= col.b )
            return col.r <= col.g ? col.r : col.g;
        else
            return col.r <= col.b ? col.r : col.b;
    }

    static uint8_t max_col( color col ) {
        if ( col.g >= col.b )
            return col.r >= col.g ? col.r : col.g;
        else
            return col.r >= col.b ? col.r : col.b;
    }

    void to_sextant_data( ) {
        h = ( h * HSV_HUE_MAX / 360 );
        s = ( s * 2.55f );
        v = ( v * 2.55f );
    }

    void swap_ptr( uint8_t *&a, uint8_t *&b ) {
        uint8_t *tmp = a;
        a = b;
        b = tmp;
    }

public:
    uint16_t h;
    uint8_t s, v;

    constexpr hsv( ) : h( 0 ), s( 0 ), v( 0 ) {}

    template< typename T1, typename T2, typename T3 >
        requires std::is_integral_v< T1 > && std::is_integral_v< T2 > && std::is_integral_v< T3 >
    constexpr hsv( const T1 &h, const T2 &s, const T3 &v ) : h( h ), s( s ), v( v ) {}

    // creds: www.vagrearg.org/content/hsvrgb
    color to_rgb( ) {
        if ( s == 0 )// grey scale
            return color( v, v, v, 255 );

        color out_col{ };
        hsv copy = *this;

        uint8_t *r = &out_col.r, *g = &out_col.g, *b = &out_col.b;

        this->to_sextant_data( );

        uint8_t sextant = h >> 8;
        if ( sextant > 5 )
            sextant = 5;

        // swap data for sextants
        if ( sextant & 2 )
            swap_ptr( r, b );

        if ( sextant & 4 )
            swap_ptr( g, b );

        if ( !( sextant & 6 ) ) {
            if ( !( sextant & 1 ) )
                swap_ptr( r, g );
        } else {
            if ( sextant & 1 )
                swap_ptr( r, g );
        }

        *g = v;

        uint16_t ww = v * ( 255 - s ) + 1;
        ww += ww >> 8;
        *b = ww >> 8;

        uint8_t h_fraction = h & 0xff;
        uint32_t d;

        if ( !( sextant & 1 ) )
            d = v * ( uint32_t ) ( ( 255 << 8 ) - ( uint16_t ) ( s * ( 256 - h_fraction ) ) );
        else
            d = v * ( uint32_t ) ( ( 255 << 8 ) - ( uint16_t ) ( s * h_fraction ) );

        d += d >> 8;
        d += v;
        *r = d >> 16;

        *this = copy;

        return out_col;
    }

    static hsv from_rgb( color from ) {
        hsv out_hsv{ };
        uint8_t rgb_min = min_col( from );
        uint8_t rgb_max = max_col( from );

        out_hsv.v = rgb_max / 2.55f;// rip, float conversion :C

        uint8_t delta = rgb_max - rgb_min;

        if ( delta == 0 )
            return out_hsv;

        out_hsv.s = ( delta * 100 ) / rgb_max;

        if ( rgb_max == from.r )
            out_hsv.h = 60 * ( from.g - from.b ) / delta + 360;
        else if ( rgb_max == from.g )
            out_hsv.h = 60 * ( from.b - from.r ) / delta + 120;
        else if ( rgb_max == from.b )
            out_hsv.h = 60 * ( from.r - from.g ) / delta + 240;

        out_hsv.h %= 360;

        return out_hsv;
    }
};

#define FMT_HEADER_ONLY
#define SPDLOG_USE_STD_FORMAT

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Types.
using vector_3d = glm::vec3;
using vector_4d = glm::vec4;
using vector_2d = glm::vec2;

struct __declspec( align( 16 ) ) quat_aligned : public glm::quat {
public:
    inline quat_aligned &operator=( const glm::quat &other ) {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        this->w = other.w;

        return *this;
    }
};

#include <core/interfaces.hpp>
#include <core/variables.hpp>
#include <sdk/interfaces/handle_entity.hpp>
#include <sdk/math/aligned_vector.hpp>
#include <sdk/math/ray.hpp>
#include <utils/math/math.hpp>
#include <sdk/other/net_msg_type.hpp>
#include <sdk/math/matrix3x4.hpp>
#include <sdk/other/networkable.hpp>
#include <sdk/other/renderable.hpp>
#include <sdk/other/thinkable.hpp>
#include <sdk/other/unknown.hpp>
#include <sdk/base_entity.hpp>
#include <sdk/cs_player.hpp>
#include <sdk/cs_base_weapon.hpp>
#include <sdk/player_animstate.hpp>
#include <sdk/user_cmd.hpp>
#include <utils/netvars/netvars.hpp>
#include <utils/datamaps/datamaps.hpp>
#include <utils/utils.hpp>
#include <utils/ray_tracer/ray_tracer.hpp>