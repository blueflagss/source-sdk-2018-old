#pragma once
#include <utils/math/math.hpp>
#include <sdk/math/aligned_vector.hpp>

struct ray_t {
    vector_3d_aligned start;
    vector_3d_aligned delta;
    vector_3d_aligned start_offset;
    vector_3d_aligned extents;
    bool is_ray;
    bool is_swept;

    ray_t( ) { }

    void init( vector_3d const &src, vector_3d const &end ) {
        delta = end - src;
        is_swept = math::length_sqr( delta ) != 0.f;
        extents = vector_3d( 0, 0, 0 );
        is_ray = true;
        start_offset = vector_3d( 0, 0, 0 );
        start = src;
    }

    void init( vector_3d const &_start, vector_3d const &end, vector_3d const &mins, vector_3d const &maxs ) {
        delta = end - _start;
        is_swept = math::length_sqr( delta ) != 0.0f;

        extents = maxs - mins;
        extents *= 0.5f;
        is_ray = math::length_sqr( extents ) < 1e-6f;

        start_offset = maxs + mins;
        start_offset *= 0.5f;
        start = _start + start_offset;
        start_offset *= -1.0f;
    }

private:
};