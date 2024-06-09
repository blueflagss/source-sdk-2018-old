#include "base_player.hpp"

bool c_base_player::compute_bounding_box( box &box_dimensions ) {
    if ( !this || !this->alive( ) ) return false;

    const auto collision_property = this->collideable( );

    if ( !collision_property )
        return false;

    auto mins = collision_property->mins( );
    auto maxs = collision_property->maxs( );

    std::array< vector_3d, 8 > points = {
            vector_3d( mins.x, mins.y, mins.z ),
            vector_3d( mins.x, maxs.y, mins.z ),
            vector_3d( maxs.x, maxs.y, mins.z ),
            vector_3d( maxs.x, mins.y, mins.z ),
            vector_3d( maxs.x, maxs.y, maxs.z ),
            vector_3d( mins.x, maxs.y, maxs.z ),
            vector_3d( mins.x, mins.y, maxs.z ),
            vector_3d( maxs.x, mins.y, maxs.z ) 
    };

    const matrix_3x4 &trans = this->renderable_to_world_transform( );

    auto left = std::numeric_limits< float >::max( );
    auto top = std::numeric_limits< float >::max( );
    auto right = std::numeric_limits< float >::lowest( );
    auto bottom = std::numeric_limits< float >::lowest( );

    std::array< vector_2d, 8 > screen_points = { };

    vector_3d transformed_points;

    for ( int i = 0; i < 8; i++ ) {
        math::vector_transform( points[ i ], trans, transformed_points );

        if ( !render::world_to_screen( transformed_points, screen_points[ i ] ) )
            return false;

        left = std::min( left, screen_points[ i ].x );
        top = std::min( top, screen_points[ i ].y );
        right = std::max( right, screen_points[ i ].x );
        bottom = std::max( bottom, screen_points[ i ].y );
    }

    box_dimensions = box(
            static_cast< float >( left ),
            static_cast< float >( top ),
            static_cast< float >( right - left ),
            static_cast< float >( bottom - top )
    );

    return true;
}

bool c_base_player::is_player_on_steam_friends( ) const {
    if ( !g_interfaces.steam_friends || !g_interfaces.steam_utils )
        return false;

    player_info_t player_info;

    if ( g_interfaces.engine_client->get_player_info( this->index( ), &player_info ) && player_info.friends_id ) {
        auto steam_id = CSteamID( player_info.friends_id, 1, k_EUniversePublic, k_EAccountTypeIndividual );

        if ( g_interfaces.steam_friends->HasFriend( steam_id, k_EFriendFlagImmediate ) )
            return true;
    }

    return false;
}

void c_base_player::pre_think( ) {
    return utils::get_method< void( __thiscall * )( void * ) >( this, 307 )( this );
}

void c_base_player::think( ) {
    return utils::get_method< void( __thiscall * )( void * ) >( this, 137 )( this );
}

void c_base_player::post_think( ) {
    return utils::get_method< void( __thiscall * )( void * ) >( this, 262 )( this );
}

void c_base_player::select_item( const char *str, int subtype ) {
    return utils::get_method< void( __thiscall * )( void *, const char *, int ) >( this, 271 )( this, str, subtype );
}

void c_base_player::update_button_state( int button_mask ) {
    static auto update_button_state = signature::find( "client.dll", "55 8B EC 8B 81 ?? ?? ?? ?? 8B D0" ).get< void( __thiscall * )( void *, int ) >( );

    return update_button_state( this, button_mask );
}