#pragma once
#include <globals.hpp>
#include <tuple>

class RayTracer {
public:
    struct Ray {
        Ray( const vector_3d &direction );
        Ray( const vector_3d &startPoint, const vector_3d &endPoint );
        vector_3d m_startPoint;
        vector_3d m_direction;
        float m_length;
    };

    struct Hitbox {
        Hitbox( );
        Hitbox( const vector_3d &mins, const vector_3d &maxs, const float radius );
        Hitbox( const std::tuple< vector_3d, vector_3d, float > &initTuple );
        vector_3d m_mins;
        vector_3d m_maxs;
        float m_radius;
    };

    struct Trace {
        Trace( );
        bool m_hit;
        float m_fraction;
        vector_3d m_traceEnd;
        vector_3d m_traceOffset;
    };

    enum Flags {
        Flags_NONE = 0,
        Flags_RETURNEND = ( 1 << 0 ),
        Flags_RETURNOFFSET = ( 1 << 1 )
    };

    // This is a specialization that starts from the center, as calculations are much simpler from the center of the hitbox
    static void TraceFromCenter( const Ray &ray, const Hitbox &hitbox, Trace &trace, int flags = 0 );
    // This is for the general case, tracing against the hitbox
    static void TraceHitbox( const Ray &ray, const Hitbox &hitbox, Trace &trace, int flags = 0 );
};

inline RayTracer g_ray_tracer{ };