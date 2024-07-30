#pragma once
#include <globals.hpp>

#define EQUAL_EPSILON 0.001

#define ADD_NETVAR( container, table, prop, tolerance ) \
    container.push_back( { g_netvars.get_offset( HASH_CT( table ), HASH_CT( prop ) ), { }, tolerance } )
#define ADD_DATAMAP_VAR( container, datamap, prop, tolerance ) \
    container.push_back( { g_datamaps.get_offset( datamap, HASH_CT( prop ) ), { }, tolerance } )

extern float assign_range_multiplier( int bits, double range );

class network_data {
private:
    inline float get_new( float old_val, float new_val, float tolerance ) {
        const float delta = new_val - old_val;
        return fabsf( delta ) <= tolerance ? old_val : new_val;
    }

    inline vector_3d get_new( vector_3d old_val, vector_3d new_val, float tolerance ) {
        return { get_new( old_val.x, new_val.x, tolerance ),
                 get_new( old_val.y, new_val.y, tolerance ),
                 get_new( old_val.z, new_val.z, tolerance ) 
        };
    }

    template< typename T >
    struct compressed_netvar {
        size_t offset{ };
        T last_value{ };
        float tolerance{ };
    };

public:
    void init( c_cs_player *player );
    void ping_reducer( );
    void pre_update( c_cs_player *player );
    void post_update( c_cs_player *player );

protected:
    bool written_pre_vars = false;
    bool initialized = false;

    std::vector< compressed_netvar< vector_3d > > vec3d_vars;
    std::vector< compressed_netvar< float > > float_vars;
};

inline network_data g_network_data = { };