#include "base_entity.hpp"

vector_3d c_base_entity::get_bone_position( const int bone, matrix_3x4 *bone_matrix ) {
    auto hitbox = bone_matrix[ bone ];

    return vector_3d( hitbox[ 0 ][ 3 ], hitbox[ 1 ][ 3 ], hitbox[ 2 ][ 3 ] );
}

c_base_entity *c_base_entity::get_root_move_parent( ) {
    auto entity = this;
    auto parent = this->get_move_parent( );

    int its = 0;

    while ( parent ) {
        if ( its > 32 ) break;
        its++;

        entity = parent;
        parent = entity->get_move_parent( );
    }

    return entity;
};

c_base_entity *c_base_entity::get_move_parent( ) {
    static auto offset = g_netvars.get_offset( HASH_CT( "DT_BaseEntity" ), HASH_CT( "moveparent" ) ) - 0x4;

    if ( !offset )
        return nullptr;

    return g_interfaces.entity_list->get_client_entity_from_handle< c_base_entity * >( *reinterpret_cast< uint32_t * >( reinterpret_cast< uintptr_t >( this ) + offset ) );
}

bool c_base_entity::update_dispatch_layer( c_animation_layer *layer, c_studio_hdr *weapon_studio_hdr, int sequence ) {
    return utils::get_method < bool( __thiscall * )( void *, c_animation_layer *, c_studio_hdr *, int ) >( this, 241 )( this, layer, weapon_studio_hdr, sequence );
}

void c_base_entity::calculate_ik_locks( float time ) {
    using CalculateIKLocks_t = void( __thiscall * )( decltype( this ), float );

#if 1
    static auto CalculateIKLocks = signature::find( _xs( "client.dll" ), _xs( "55 8B EC 83 E4 F8 81 ? ? ? ? ? 56 57 8B F9 89 7C 24 18" ) ).get< CalculateIKLocks_t >( );
#else
    static auto CalculateIKLocks = signature::find( _xs( "server.dll" ), _xs( "55 8B EC 83 E4 F8 81 ? ? ? ? ? 56 57 8B F9 89 7C 24 18" ) ).get< CalculateIKLocks_t >( );
#endif

    CalculateIKLocks( this, time );
}

bool c_base_entity::get_hitbox_position( const int hit_group, vector_3d &position ) {
    auto model = this->get_model( );

    if ( !model )
        return false;

    auto studio_hdr = g_interfaces.model_info->get_studio_model( model );

    if ( !studio_hdr )
        return false;

    auto hitbox_set = studio_hdr->hitbox_set( this->hitbox_set( ) );

    if ( !hitbox_set )
        return false;

    matrix_3x4 matrix[ 128 ];

    if ( !this->setup_bones( matrix, 128, 0x100, g_interfaces.global_vars->curtime ) )
        return false;

    mstudiobbox_t *final_box = nullptr;

    for ( int n = 0; n < hitbox_set->num_hitboxes; n++ ) {
        auto box = hitbox_set->hitbox( n );

        if ( !box || ( box->group != hit_group ) || ( box->bone < 0 ) || ( box->bone >= 128 ) )
            continue;

        final_box = box;
    }

    if ( !final_box )
        return false;

    math::vector_transform( ( final_box->min + final_box->max ) * 0.5f, matrix[ final_box->bone ], position );

    return true;
}

void c_base_entity::set_sequence( int sequence ) {
    return utils::get_method< void( __thiscall * )( void *, int ) >( this, 213 )( this, sequence );
}

void c_base_entity::set_abs_origin( const vector_3d &origin ) {
    static auto set_abs_origin = signature::find( "client.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8 ? ? ? ? 8B 7D" ).get< void( __thiscall * )( void *, const vector_3d & ) >( );

    return set_abs_origin( this, origin );
}

void c_base_entity::invalidate_physics_recursive( int change_flags ) {
    static auto invalidate_physics_recursive = signature::find( "client.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56" ).get< void( __thiscall * )( void *, int ) >( );

    return invalidate_physics_recursive( this, change_flags );
}

void c_base_entity::set_abs_angles( const vector_3d &angles ) {
    static auto set_abs_angles = signature::find( "client.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1" ).get< void( __thiscall * )( void *, const vector_3d & ) >( );

    return set_abs_angles( this, angles );
}

void c_base_entity::set_collision_bounds( const vector_3d &mins, const vector_3d &maxs ) {
    static auto set_collision_bound = signature::find( "client.dll", "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 10 56 57 8B 7B" ).get< void( __thiscall * )( void *, const vector_3d &, const vector_3d & ) >( );

    return set_collision_bound( this->collideable( ), mins, maxs );
}

bool c_base_entity::physics_run_think( int think_method ) {
    static auto physics_run_think = signature::find( "client.dll", "55 8B EC 83 EC 10 53 56 57 8B F9 8B 87 ? ? ? ? C1" ).get< bool( __thiscall * )( void *, int ) >( );

    return physics_run_think( this, think_method );
}

void c_base_entity::set_next_think( int context_index, float think_time ) {
    static auto set_next_think = signature::find( "client.dll", "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6 C1 E8 16 24 01 74 18" ).get< void( __thiscall * )( void *, int, float ) >( );

    return set_next_think( this, context_index, think_time );
}

void *c_base_entity::get_predicted_frame( int framenumber ) {
    static auto get_predicted_frame = signature ::find( "client.dll", "55 8B EC 57 8B F9 83 BF EC" ).get< void *( __thiscall * ) ( void *, int ) >( );

    return get_predicted_frame( this, framenumber );
}