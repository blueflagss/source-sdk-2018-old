#include "matrix3x4.hpp"
#include <utils/math/math.hpp>

vector_3d matrix_3x4::operator*( const vector_3d &vec ) const {
    auto &m = mat;

    vector_3d ret;
    ret.x = m[ 0 ][ 0 ] * vec.x + m[ 0 ][ 1 ] * vec.y + m[ 0 ][ 2 ] * vec.z + m[ 0 ][ 3 ];
    ret.y = m[ 1 ][ 0 ] * vec.x + m[ 1 ][ 1 ] * vec.y + m[ 1 ][ 2 ] * vec.z + m[ 1 ][ 3 ];
    ret.z = m[ 2 ][ 0 ] * vec.x + m[ 2 ][ 1 ] * vec.y + m[ 2 ][ 2 ] * vec.z + m[ 2 ][ 3 ];

    return ret;
}

matrix_3x4 matrix_3x4::operator*( const matrix_3x4 &vm ) {
    matrix_3x4 out;

    math::concat_transforms( *this, vm, out );

    return out;
}