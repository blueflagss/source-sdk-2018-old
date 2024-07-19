#include "math.hpp"

void math::matrix_get_column( const matrix_3x4 &in, int column, vector_3d &out ) {
    out.x = in[ 0 ][ column ];
    out.y = in[ 1 ][ column ];
    out.z = in[ 2 ][ column ];
}

void math::matrix_copy( const matrix_3x4 &in, matrix_3x4 &out ) {
    std::memcpy( out.base( ), in.base( ), sizeof( matrix_3x4 ) );
}

void math::vector_rotate( const vector_3d &in1, matrix_3x4 in2, vector_3d &out ) {
    out.x = glm::dot( in1, (vector_3d)*in2[ 0 ] );
    out.y = glm::dot( in1, (vector_3d)*in2[ 1 ] );
    out.z = glm::dot( in1, (vector_3d)*in2[ 2 ] );
}

vector_3d math::vector_rotate( const vector_3d &in1, const vector_3d &in2 ) {
    matrix_3x4 matrix;
    angle_matrix( in2, matrix );

    vector_3d out;
    vector_rotate( in1, matrix, out );

    return out;
}

bool math::intersect_ray_with_box( const vector_3d &rayStart, const vector_3d &rayDelta, const vector_3d &boxMins, const vector_3d &boxMaxs, float epsilon, c_game_trace *pTrace, float *pFractionLeftSolid ) {
    static auto fn = signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 83 C4 14 84 C0 74 3F" ) ).add( 0x1 ).rel32( ).get< bool( __fastcall * )( const vector_3d &, const vector_3d &, const vector_3d &, const vector_3d &, float, c_game_trace *, float * ) >( );
    return fn( rayStart, rayDelta, boxMins, boxMaxs, epsilon, pTrace, pFractionLeftSolid );
}

bool math::intersect_bb( vector_3d &start, vector_3d &delta, vector_3d &min, vector_3d &max ) {
    float d1, d2, f;
    bool start_solid = true;
    float t1 = -1.0f, t2 = 1.0f;

    const float _start[ 3 ] = { start.x, start.y, start.z };
    const float _delta[ 3 ] = { delta.x, delta.y, delta.z };
    const float mins[ 3 ] = { min.x, min.y, min.z };
    const float maxs[ 3 ] = { max.x, max.y, max.z };

    for ( auto i = 0; i < 6; ++i ) {
        if ( i >= 3 ) {
            const auto j = ( i - 3 );

            d1 = _start[ j ] - maxs[ j ];
            d2 = d1 + _delta[ j ];
        } else {
            d1 = -_start[ i ] + mins[ i ];
            d2 = d1 - _delta[ i ];
        }

        if ( d1 > 0 && d2 > 0 ) {
            start_solid = false;
            return false;
        }

        if ( d1 <= 0 && d2 <= 0 )
            continue;

        if ( d1 > 0 )
            start_solid = false;

        if ( d1 > d2 ) {
            f = d1;
            if ( f < 0 )
                f = 0;

            f /= d1 - d2;
            if ( f > t1 )
                t1 = f;
        } else {
            f = d1 / ( d1 - d2 );
            if ( f < t2 )
                t2 = f;
        }
    }

    return start_solid || ( t1 < t2 && t1 >= 0.0f );
}
float math::segment_to_segment( const vector_3d s1, const vector_3d s2, const vector_3d k1, const vector_3d k2 ) {
    static auto constexpr epsilon = 0.00000001;

    auto u = s2 - s1;
    auto v = k2 - k1;
    const auto w = s1 - k1;

    const auto a = glm::dot( u, u );
    const auto b = glm::dot( u, v );
    const auto c = glm::dot( v, v );
    const auto d = glm::dot( u, w );
    const auto e = glm::dot( v, w );
    const auto D = a * c - b * b;
    float sn, sd = D;
    float tn, td = D;

    if ( D < epsilon ) {
        sn = 0.0;
        sd = 1.0;
        tn = e;
        td = c;
    } else {
        sn = b * e - c * d;
        tn = a * e - b * d;

        if ( sn < 0.0 ) {
            sn = 0.0;
            tn = e;
            td = c;
        } else if ( sn > sd ) {
            sn = sd;
            tn = e + b;
            td = c;
        }
    }

    if ( tn < 0.0 ) {
        tn = 0.0;

        if ( -d < 0.0 )
            sn = 0.0;
        else if ( -d > a )
            sn = sd;
        else {
            sn = -d;
            sd = a;
        }
    } else if ( tn > td ) {
        tn = td;

        if ( -d + b < 0.0 )
            sn = 0;
        else if ( -d + b > a )
            sn = sd;
        else {
            sn = -d + b;
            sd = a;
        }
    }

    const float sc = abs( sn ) < epsilon ? 0.0f : sn / sd;
    const float tc = abs( tn ) < epsilon ? 0.0f : tn / td;

    m128 n;
    auto dp = w + u * sc - v * tc;
    n.f[ 0 ] = glm::dot( dp, dp );
    const auto calc = sqrt_ps( n.v );
    return reinterpret_cast< const m128 * >( &calc )->f[ 0 ];
}
bool math::intersect( vector_3d start, vector_3d end, vector_3d a, vector_3d b, float radius ) {
    const auto dist = segment_to_segment( start, end, a, b );
    return ( dist < radius );
}

void math::vector_irotate( const vector_3d &in1, const matrix_3x4 &in2, vector_3d &out ) {
    out.x = in1.x * in2[ 0 ][ 0 ] + in1.y * in2[ 1 ][ 0 ] + in1.z * in2[ 2 ][ 0 ];
    out.y = in1.x * in2[ 0 ][ 1 ] + in1.y * in2[ 1 ][ 1 ] + in1.z * in2[ 2 ][ 1 ];
    out.z = in1.x * in2[ 0 ][ 2 ] + in1.y * in2[ 1 ][ 2 ] + in1.z * in2[ 2 ][ 2 ];
}

void math::concat_transforms( const matrix_3x4& in1, const matrix_3x4& in2, matrix_3x4& out ) {
    if ( &in1 == &out ) {
        matrix_3x4 in1b;
        matrix_copy( in1, in1b );
        concat_transforms( in1b, in2, out );
        return;
    }

    if ( &in2 == &out ) {
        matrix_3x4 in2b;
        matrix_copy( in2, in2b );
        concat_transforms( in1, in2b, out );
        return;
    }

    out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] + in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
    out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] + in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
    out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] + in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
    out[ 0 ][ 3 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 3 ] + in1[ 0 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 0 ][ 3 ];

    out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] + in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
    out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] + in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
    out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] + in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
    out[ 1 ][ 3 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 3 ] + in1[ 1 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 1 ][ 3 ];

    out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] + in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
    out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] + in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
    out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] + in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
    out[ 2 ][ 3 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 3 ] + in1[ 2 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 2 ][ 3 ];
}

void math::angle_matrix( const vector_3d &angles, matrix_3x4 &matrix ) {
    float sr, sp, sy, cr, cp, cy;

    sin_cos( deg_to_rad( angles[ 1 ] ), &sy, &cy );
    sin_cos( deg_to_rad( angles[ 0 ] ), &sp, &cp );
    sin_cos( deg_to_rad( angles[ 2 ] ), &sr, &cr );

    matrix[ 0 ][ 0 ] = cp * cy;
    matrix[ 1 ][ 0 ] = cp * sy;
    matrix[ 2 ][ 0 ] = -sp;

    matrix[ 0 ][ 1 ] = sr * sp * cy + cr * -sy;
    matrix[ 1 ][ 1 ] = sr * sp * sy + cr * cy;
    matrix[ 2 ][ 1 ] = sr * cp;
    matrix[ 0 ][ 2 ] = ( cr * sp * cy + -sr * -sy );
    matrix[ 1 ][ 2 ] = ( cr * sp * sy + -sr * cy );
    matrix[ 2 ][ 2 ] = cr * cp;

    matrix[ 0 ][ 3 ] = 0.0f;
    matrix[ 1 ][ 3 ] = 0.0f;
    matrix[ 2 ][ 3 ] = 0.0f;
}

float math::dist_segment_to_segment_sqr( const vector_3d &p1, const vector_3d &p2, const vector_3d &q1, const vector_3d &q2, float &invariant1, float &invariant2 ) {
    static const auto kSmallNumber = 0.0001f;

    const auto u = p2 - p1;
    const auto v = q2 - q1;
    const auto w = p1 - q1;
    const auto a = glm::dot(u, u );
    const auto b = glm::dot(u, v );
    const auto c = glm::dot( v, v );
    const auto d = glm::dot( u, w );
    const auto e = glm::dot( v, w );
    const auto f = a * c - b * b;

    // s1,s2 and t1,t2 are the parametric representation of the intersection.
    // they will be the invariants at the end of this simple computation.

    float s1;
    auto s2 = f;
    float t1;
    auto t2 = f;

    if ( f < kSmallNumber ) {
        s1 = 0.0;
        s2 = 1.0;
        t1 = e;
        t2 = c;

    } else {
        s1 = ( b * e - c * d );
        t1 = ( a * e - b * d );
        if ( s1 < 0.0 ) {
            s1 = 0.0;
            t1 = e;
            t2 = c;

        } else if ( s1 > s2 ) {
            s1 = s2;
            t1 = e + b;
            t2 = c;
        }
    }

    if ( t1 < 0.0f ) {
        t1 = 0.0f;
        if ( -d < 0.0f )
            s1 = 0.0f;
        else if ( -d > a )
            s1 = s2;
        else {
            s1 = -d;
            s2 = a;
        }

    } else if ( t1 > t2 ) {
        t1 = t2;
        if ( ( -d + b ) < 0.0f )
            s1 = 0;
        else if ( ( -d + b ) > a )
            s1 = s2;
        else {
            s1 = ( -d + b );
            s2 = a;
        }
    }

    invariant1 = ( ( std::abs( s1 ) < kSmallNumber ) ? 0.0f : s1 / s2 );
    invariant2 = ( std::abs( t1 ) < kSmallNumber ? 0.0f : t1 / t2 );

    return length_sqr( w + ( u * invariant1 ) - ( v * invariant2 ) );
}

float math::distance_to_ray( const vector_3d &pos, const vector_3d &ray_start, const vector_3d &ray_end, float *along, vector_3d *point_on_ray ) {
    const vector_3d to = ( pos - ray_start );
    vector_3d dir = ( ray_end - ray_start );

    float length = normalize_place( dir );
    float range_along = glm::dot( dir, to );

    if ( along )
        *along = range_along;

    float range;

    if ( range_along < 0.0f )
        range = -( to ).length( );
    else if ( range_along > length )
        range = -( pos - ray_end ).length( );
    else
        range = ( pos - ( dir * range_along + ray_start ) ).length( );

    return range;
}

float math::normalize_place( vector_3d other ) {
    auto radius = std::sqrtf( other.x * other.x + other.y * other.y + other.z * other.z );
    auto iradius = 1.0f / ( radius + std::numeric_limits< float >::epsilon( ) );

    other.x *= iradius;
    other.y *= iradius;
    other.z *= iradius;

    return radius;
}

vector_3d math::velocity_to_angles( const vector_3d &direction ) {
    auto magnitude = [ & ]( const vector_3d &v ) -> float {
        return std::sqrtf( math::dot_product( v, v ) );
    };

    float yaw, pitch;

    if ( direction.y == 0.0f && direction.x == 0.0f ) {
        yaw = 0.0f;

        if ( direction.z > 0.0f )
            pitch = 270.0f;
        else
            pitch = 90.0f;
    }

    else {
        yaw = rad_to_deg( std::atan2f( direction.y, direction.x ) );
        pitch = rad_to_deg( std::atan2f( -direction.z, magnitude( vector_3d( direction ) ) ) );

        if ( yaw < 0.0f )
            yaw += 360.0f;

        if ( pitch < 0.0f )
            pitch += 360.0f;
    }

    return { pitch, yaw, 0.0f };
}

void math::matrix_position( const matrix_3x4 &matrix, vector_3d &position ) {
    matrix_get_column( matrix, 3, position );
}

void math::angle_vectors( vector_3d angles, vector_3d *forward ) {
    auto clamped = clamp_angle( angles );

    float sp, sy, cp, cy;

    sin_cos( deg_to_rad( clamped.y ), &sy, &cy );
    sin_cos( deg_to_rad( clamped.x ), &sp, &cp );

    forward->x = cp * cy;
    forward->y = cp * sy;
    forward->z = -sp;
}

vector_3d math::angle_from_vectors( vector_3d a, vector_3d b ) {
    vector_3d angles{ };

    vector_3d delta = a - b;
    float hyp = glm::length( delta );

    // 57.295f - pi in degrees
    angles.y = std::atan( delta.y / delta.x ) * 57.2957795131f;
    angles.x = std::atan( -delta.z / hyp ) * -57.2957795131f;
    angles.z = 0.0f;

    if ( delta.x >= 0.0f )
        angles.y += 180.0f;

    return angles;
}

vector_3d math::angle_vectors( vector_3d angles ) {
    vector_3d forward;

    auto clamped = clamp_angle( angles );
    float sp, sy, cp, cy;

    sin_cos( deg_to_rad( clamped.y ), &sy, &cy );
    sin_cos( deg_to_rad( clamped.x ), &sp, &cp );

    forward.x = cp * cy;
    forward.y = cp * sy;
    forward.z = -sp;

    return forward;
}

vector_3d math::clamp_angle( const vector_3d &angle ) {
    vector_3d return_value;

    auto flt_valid = [ & ]( float val ) {
        return std::isfinite( val ) && !std::isnan( val );
    };

    for ( auto i = 0; i < 3; i++ )
        if ( !flt_valid( angle[ i ] ) )
            return_value[ i ] = 0.0f;

    return_value.x = std::clamp< float >( normalize( angle.x ), -89.0f, 89.0f );
    return_value.y = std::clamp< float >( normalize( angle.y ), -180.0f, 180.0f );
    return_value.z = 0.0f;

    return return_value;
}

float math::calculate_fov( const vector_3d &start, const vector_3d &end ) {
    vector_3d angle, aim;

    angle_vectors( start, &angle );
    angle_vectors( end, &aim );

    return rad_to_deg( acosf( math::dot_product( aim, angle ) / math::length_sqr( aim ) ) );
}

void math::rotate_point( int &x, int &y, float rotation ) {
    auto temp_x = x;
    auto temp_y = y;

    float theta = deg_to_rad( rotation );
    float c = cos( theta );
    float s = sin( theta );

    x = ( int ) ( ( ( float ) temp_x * c ) - ( ( float ) temp_y * s ) );
    y = ( int ) ( ( ( float ) temp_x * s ) + ( ( float ) temp_y * c ) );
}

void math::rotate_point( glm::vec2 &point, float rotation ) {
    int x, y;

    x = point.x, y = point.y;
    rotate_point( x, y, rotation );
    point = { x, y };// shitty other function won't let me pass vector as a reference.
}

void math::random_seed( int seed ) {
    static auto random_seed = reinterpret_cast< void( __cdecl * )( int ) >( GetProcAddress( GetModuleHandleA( _xs( "vstdlib.dll" ) ), _xs( "RandomSeed" ) ) );

    random_seed( seed );
}

float math::random_float( float min_val, float max_value ) {
    static auto random_float = reinterpret_cast< float( __cdecl * )( float, float ) >( GetProcAddress( GetModuleHandleA( _xs( "vstdlib.dll" ) ), _xs( "RandomFloat" ) ) );

    return random_float( min_val, max_value );
}

void math::angle_normalize( float &angle ) {
    float rot;

    // bad number.
    if ( !std::isfinite( angle ) ) {
        angle = 0.f;
        return;
    }

    // no need to normalize this angle.
    if ( angle >= -180.f && angle <= 180.f )
        return;

    // get amount of rotations needed.
    rot = std::round( std::abs( angle / 360.f ) );

    // normalize.
    angle = ( angle < 0.f ) ? angle + ( 360.f * rot ) : angle - ( 360.f * rot );
}

vector_3d math::normalize_angle( vector_3d angle ) {
    auto vec = std::sqrt( length_sqr( angle ) );

    angle /= ( vec + std::numeric_limits< float >::epsilon( ) );

    return angle;
}

void math::vector_transform( const vector_3d in1, const matrix_3x4 &in2, vector_3d &out ) {
    out[ 0 ] = math::dot_product( in1, { in2[ 0 ][ 0 ], in2[ 0 ][ 1 ], in2[ 0 ][ 2 ] } ) + in2[ 0 ][ 3 ];
    out[ 1 ] = math::dot_product( in1, { in2[ 1 ][ 0 ], in2[ 1 ][ 1 ], in2[ 1 ][ 2 ] } ) + in2[ 1 ][ 3 ];
    out[ 2 ] = math::dot_product( in1, { in2[ 2 ][ 0 ], in2[ 2 ][ 1 ], in2[ 2 ][ 2 ] } ) + in2[ 2 ][ 3 ];
}

vector_3d math::vector_transform( const vector_3d in1, const matrix_3x4 &in2 ) {
    vector_3d out;

    out[ 0 ] = math::dot_product( in1, { in2[ 0 ][ 0 ], in2[ 0 ][ 1 ], in2[ 0 ][ 2 ] } ) + in2[ 0 ][ 3 ];
    out[ 1 ] = math::dot_product( in1, { in2[ 1 ][ 0 ], in2[ 1 ][ 1 ], in2[ 1 ][ 2 ] } ) + in2[ 1 ][ 3 ];
    out[ 2 ] = math::dot_product( in1, { in2[ 2 ][ 0 ], in2[ 2 ][ 1 ], in2[ 2 ][ 2 ] } ) + in2[ 2 ][ 3 ];

    return out;
}

void math::sin_cos( float radians, float *sine, float *cosine ) {
    *sine = sin( radians );
    *cosine = cos( radians );
}

vector_3d math::vector_angle( const vector_3d &position ) {
    float tmp, yaw, pitch;
    vector_3d angles;

    if ( position.y == 0 && position.x == 0 ) {
        yaw = 0;

        if ( position.z > 0 )
            pitch = 270;
        else
            pitch = 90;
    }

    else {
        yaw = rad_to_deg( atan2f( position.y, position.x ) );

        if ( yaw < 0 )
            yaw += 360;

        tmp = math::length_2d( position );
        pitch = rad_to_deg( atan2f( -position.z, tmp ) );

        if ( pitch < 0 )
            pitch += 360;
    }

    angles[ 0 ] = pitch;
    angles[ 1 ] = yaw;
    angles[ 2 ] = 0;

    return angles;
}

vector_3d math::calculate_angle( const vector_3d &source, const vector_3d &destination ) {
    vector_3d delta = source - destination;
    vector_3d angles;

    angles.x = math::rad_to_deg( asinf( delta.z / delta.length( ) ) );
    angles.y = math::rad_to_deg( atanf( delta.y / delta.x ) );
    angles.z = 0.0f;

    if ( delta.x >= 0.0 )
        angles.y += 180.0f;

    return angles;
}

void math::angle_vectors( const vector_3d &angles, vector_3d *forward, vector_3d *right, vector_3d *up ) {
    float cp = std::cos( deg_to_rad( angles.x ) ), sp = std::sin( deg_to_rad( angles.x ) );
    float cy = std::cos( deg_to_rad( angles.y ) ), sy = std::sin( deg_to_rad( angles.y ) );
    float cr = std::cos( deg_to_rad( angles.z ) ), sr = std::sin( deg_to_rad( angles.z ) );

    if ( forward ) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    if ( right ) {
        right->x = ( -1 * sr * sp * cy + -1 * cr * -sy );
        right->y = ( -1 * sr * sp * sy + -1 * cr * cy );
        right->z = -1 * sr * cp;
    }

    if ( up ) {
        up->x = ( cr * sp * cy + -sr * -sy );
        up->y = ( cr * sp * sy + -sr * cy );
        up->z = cr * cp;
    }
}

void math::points_from_box( const vector_3d mins, const vector_3d maxs, vector_3d *points ) {
    for ( int n = 0; n < 6; n++ ) {
        points[ n ][ 0 ] = mins[ 0 ];
        points[ n ][ 1 ] = mins[ 1 ];
        points[ n ][ 2 ] = mins[ 2 ];
    }
}

float math::normalize( float angle ) {
    if ( std::isnan( angle ) || std::isinf( angle ) )
        angle = 0.0f;

    while ( angle > 180.0f )
        angle -= 360.0f;

    while ( angle < -180.0f )
        angle += 360.0f;

    return angle;
}

float math::dot_product( const vector_3d &a, const vector_3d &b ) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float math::length_2d( const vector_3d &a ) {
    return std::sqrtf( a.x * a.x + a.y * a.y );
}

float math::length_sqr( const vector_3d &in ) {
    return in.x * in.x + in.y * in.y + in.z * in.z;
}

float math::length_2d_sqr( const vector_3d &in ) {
    return ( in.x * in.x + in.y * in.y );
}