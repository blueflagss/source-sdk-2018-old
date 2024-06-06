#pragma once

class __declspec( align( 16 ) ) vector_3d_aligned : public vector_3d {
public:
    inline vector_3d_aligned( ) {}

    inline void init( ) {
        this->x = 0.0f;
        this->y = 0.0f;
        this->z = 0.0f;
    }

    inline void init( float x, float y, float z ) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    inline vector_3d_aligned( float x, float y, float z ) {
        init( x, y, z );
    }

    explicit vector_3d_aligned( const vector_3d &other ) {
        init( other.x, other.y, other.z );
    }

    vector_3d_aligned &operator=( const vector_3d &other ) {
        init( other.x, other.y, other.z );
        return *this;
    }

    float w;
};
