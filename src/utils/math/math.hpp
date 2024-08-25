#pragma once
#include <globals.hpp>
#include <glm/gtx/norm.hpp>

struct box_t {
    box_t( ) = default;
    box_t( float x, float y, float w, float h ) : x( x ), y( y ), w( w ), h( h ){ };

    float x, y, w, h;
};

typedef __declspec( align( 16 ) ) union {
    float f[ 4 ];
    uint32_t u[ 4 ];
    __m128 v;
} m128;

class matrix_3x4;
class c_game_trace;

inline __m128 sqrt_ps( const __m128 squared ) {
    return _mm_sqrt_ps( squared );
}

namespace math
{
    inline constexpr float deg_to_rad( float x ) {
        return x * ( glm::pi< float >( ) / 180.f );
    }

    inline constexpr float rad_to_deg( float x ) {
        return x * ( 180.f / glm::pi< float >( ) );
    }

    inline float normalize_angle( float angle, float start = -180.f, float end = 180.f ) {
        const float width = end - start;
        const float offset_angle = angle - start;

        return ( offset_angle - ( floor( offset_angle / width ) * width ) ) + start;
    }

    inline vector_3d lerp_vector( vector_3d from, vector_3d to, float fraction ) {
        return {
            std::lerp< float >( from.x, to.x, fraction ),
            std::lerp< float >( from.y, to.y, fraction ),
            std::lerp< float >( from.z, to.z, fraction ),
        };
    }

    inline vector_3d interpolate(const vector_3d& from, const vector_3d& to, const float percent) {
        return to * percent + from * ( 1.f - percent );
    }

    inline float dist_sqr(const vector_3d& first, const vector_3d& other) {
        vector_3d delta( first.x - other.x, first.y - other.y, first.z - other.z );
        return glm::length2( delta );
    }

    void angle_normalize( float &angle );
    void rotate_point( int &x, int &y, float rotation );
    void rotate_point( glm::vec2 &point, float rotation );
    void random_seed( int seed );
    float random_float( float min_val, float max_value );
    vector_3d normalize_vector( const vector_3d &val );
    vector_3d normalize_angle( vector_3d angle );
    void vector_transform( const vector_3d in1, const matrix_3x4 &in2, vector_3d &out );
    vector_3d vector_transform( const vector_3d in1, const matrix_3x4 &in2 );
    void angle_vectors( vector_3d angles, vector_3d *forward );
    vector_3d angle_from_vectors( vector_3d a, vector_3d b );
    vector_3d angle_vectors( vector_3d angles );
    void sin_cos( float radians, float *sine, float *cosine );
    void angle_vectors( const vector_3d &angles, vector_3d *forward, vector_3d *right, vector_3d *up );
    float dot_product( const vector_3d &a, const vector_3d &b );
    float length_sqr( const vector_3d &in );
    float length_2d_sqr( const vector_3d &in );
    float length_2d( const vector_3d &a );
    vector_3d vector_angle( const vector_3d &position );
    matrix_3x4 setup_matrix_scale( const vector_3d &scale );
    void matrix_multiply( const matrix_3x4 &src1, const matrix_3x4 &src2, matrix_3x4 &dst );
    vector_3d calculate_angle( const vector_3d &source, const vector_3d &destination );
    float calculate_fov( const vector_3d &start, const vector_3d &destination );
    void matrix_position( const matrix_3x4 &matrix, vector_3d &position );
    void matrix_get_column( const matrix_3x4 &in, int column, vector_3d &out );
    void vector_scale( const float *in, float scale, float *out );
    void matrix_set_column( const vector_3d &in, int column, matrix_3x4 &out );
    void matrix_copy( const matrix_3x4 &in, matrix_3x4 &out );
    void vector_rotate( const vector_3d &in1, matrix_3x4 in2, vector_3d &out );
    vector_3d vector_rotate( const vector_3d &in1, const vector_3d &in2 );
    void angle_matrix( const vector_3d &ang, const vector_3d &pos, matrix_3x4 &out );
    bool intersect_ray_with_box( const vector_3d &rayStart, const vector_3d &rayDelta, const vector_3d &boxMins, const vector_3d &boxMaxs, float epsilon, c_game_trace *pTrace, float *pFractionLeftSolid );
    bool intersect_bb( vector_3d &start, vector_3d &delta, vector_3d &min, vector_3d &max );
    float segment_to_segment( const vector_3d &s1, const vector_3d &s2, const vector_3d &k1, const vector_3d &k2 );
    void quaternion_matrix( const vector_4d &q, const vector_3d &pos, matrix_3x4 &matrix );
    void quaternion_matrix( const vector_4d &q, const vector_3d &pos, const vector_3d &vScale, matrix_3x4 &mat );
    void quaternion_matrix( const vector_4d &q, matrix_3x4 &matrix );
    bool intersect( vector_3d &start, vector_3d &end, vector_3d &a, vector_3d &b, float radius );
    void vector_irotate( const vector_3d &in1, const matrix_3x4 &in2, vector_3d &out );
    void concat_transforms( const matrix_3x4 &in1, const matrix_3x4 &in2, matrix_3x4 &out );
    void angle_matrix( const vector_3d &angles, matrix_3x4 &matrix );
    float dist_segment_to_segment_sqr( const vector_3d &p1, const vector_3d &p2, const vector_3d &q1, const vector_3d &q2, float &invariant1, float &invariant2 );
    float distance_to_ray( const vector_3d &pos, const vector_3d &ray_start, const vector_3d &ray_end, float *along = 0, vector_3d *point_on_ray = 0 );
    float normalize_place( vector_3d other );
    vector_3d velocity_to_angles( const vector_3d &direction );
    vector_3d clamp_angle( const vector_3d &angle );
    float normalize( float angle );
    void points_from_box( const vector_3d mins, const vector_3d maxs, vector_3d *points );
    float calculate_fov_from_position( const vector_3d &from, const vector_3d &to );
    float calculate_fov_from_position( const vector_3d &view_angles, const vector_3d &start, const vector_3d &end );
}// namespace math