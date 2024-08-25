#pragma once
#include <precompiled.hpp>

class matrix_3x4 {
public:
    matrix_3x4( ) {}

    matrix_3x4( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23 ) {
        mat[ 0 ][ 0 ] = m00;
        mat[ 0 ][ 1 ] = m01;
        mat[ 0 ][ 2 ] = m02;
        mat[ 0 ][ 3 ] = m03;
        mat[ 1 ][ 0 ] = m10;
        mat[ 1 ][ 1 ] = m11;
        mat[ 1 ][ 2 ] = m12;
        mat[ 1 ][ 3 ] = m13;
        mat[ 2 ][ 0 ] = m20;
        mat[ 2 ][ 1 ] = m21;
        mat[ 2 ][ 2 ] = m22;
        mat[ 2 ][ 3 ] = m23;
    }
    inline void Init(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33 ) {
        mat[ 0 ][ 0 ] = m00;
        mat[ 0 ][ 1 ] = m01;
        mat[ 0 ][ 2 ] = m02;
        mat[ 0 ][ 3 ] = m03;
        
        mat[ 1 ][ 0 ] = m10;
        mat[ 1 ][ 1 ] = m11;
        mat[ 1 ][ 2 ] = m12;
        mat[ 1 ][ 3 ] = m13;
        
        mat[ 2 ][ 0 ] = m20;
        mat[ 2 ][ 1 ] = m21;
        mat[ 2 ][ 2 ] = m22;
        mat[ 2 ][ 3 ] = m23;
        
        mat[ 3 ][ 0 ] = m30;
        mat[ 3 ][ 1 ] = m31;
        mat[ 3 ][ 2 ] = m32;
        mat[ 3 ][ 3 ] = m33;
    }


    //-----------------------------------------------------------------------------
    // Initialize from a 3x4
    //-----------------------------------------------------------------------------
    inline void Init( const matrix_3x4 &m ) {
        memcpy( mat, m.base( ), sizeof( matrix_3x4 ) );

        mat[ 3 ][ 0 ] = 0.0f;
        mat[ 3 ][ 1 ] = 0.0f;
        mat[ 3 ][ 2 ] = 0.0f;
        mat[ 3 ][ 3 ] = 1.0f;
    }

    void init( const vector_3d &x, const vector_3d &y, const vector_3d &z, const vector_3d &origin ) {
        mat[ 0 ][ 0 ] = x.x;
        mat[ 0 ][ 1 ] = y.x;
        mat[ 0 ][ 2 ] = z.x;
        mat[ 0 ][ 3 ] = origin.x;
        mat[ 1 ][ 0 ] = x.y;
        mat[ 1 ][ 1 ] = y.y;
        mat[ 1 ][ 2 ] = z.y;
        mat[ 1 ][ 3 ] = origin.y;
        mat[ 2 ][ 0 ] = x.z;
        mat[ 2 ][ 1 ] = y.z;
        mat[ 2 ][ 2 ] = z.z;
        mat[ 2 ][ 3 ] = origin.z;
    }

    matrix_3x4( const vector_3d &x, const vector_3d &y, const vector_3d &z, const vector_3d &origin ) {
        mat[ 0 ][ 0 ] = x.x;
        mat[ 0 ][ 1 ] = y.x;
        mat[ 0 ][ 2 ] = z.x;
        mat[ 0 ][ 3 ] = origin.x;
        mat[ 1 ][ 0 ] = x.y;
        mat[ 1 ][ 1 ] = y.y;
        mat[ 1 ][ 2 ] = z.y;
        mat[ 1 ][ 3 ] = origin.y;
        mat[ 2 ][ 0 ] = x.z;
        mat[ 2 ][ 1 ] = y.z;
        mat[ 2 ][ 2 ] = z.z;
        mat[ 2 ][ 3 ] = origin.z;
    }

    void set_origin( const vector_3d &p ) {
        mat[ 0 ][ 3 ] = p.x;
        mat[ 1 ][ 3 ] = p.y;
        mat[ 2 ][ 3 ] = p.z;
    }

    vector_3d get_origin( ) {
        return { mat[ 0 ][ 3 ], mat[ 1 ][ 3 ], mat[ 2 ][ 3 ] };
    }

    float *operator[]( int i ) {
        return mat[ i ];
    }

    const float *operator[]( int i ) const {
        return mat[ i ];
    }

    float *base( ) {
        return &mat[ 0 ][ 0 ];
    }

    const float *base( ) const {
        return &mat[ 0 ][ 0 ];
    }

     vector_3d operator*( const vector_3d &vec ) const;
    matrix_3x4 operator+( const matrix_3x4 &other ) const;
    matrix_3x4 operator*( const float &other ) const;
    matrix_3x4 operator*( const matrix_3x4 &vm );

public:
    float mat[ 3 ][ 4 ];
};

inline matrix_3x4 matrix_3x4::operator+( const matrix_3x4 &other ) const {
    matrix_3x4 ret;
    auto &m = mat;
    for ( int i = 0; i < 12; i++ ) {
        ( ( float * ) ret.mat )[ i ] = ( ( float * ) m )[ i ] + ( ( float * ) other.mat )[ i ];
    }
    return ret;
}

inline matrix_3x4 matrix_3x4::operator*( const float &other ) const {
    matrix_3x4 ret;
    auto &m = mat;
    for ( int i = 0; i < 12; i++ ) {
        ( ( float * ) ret.mat )[ i ] = ( ( float * ) m )[ i ] * other;
    }
    return ret;
}

class v_matrix {
private:
    vector_3d m[ 4 ][ 4 ];

public:
    __inline const matrix_3x4 &as_3x4( ) const {
        return *( ( const matrix_3x4 * ) this );
    }
};

class __declspec( align( 16 ) ) matrix_3x4_aligned : public matrix_3x4 {
public:
    __inline matrix_3x4_aligned &operator=( const matrix_3x4 &src ) {
        memcpy( base( ), src.base( ), sizeof( float ) * 3 * 4 );
        return *this;
    };
};