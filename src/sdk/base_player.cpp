#include "base_player.hpp"
#include <features/animations/animation_sync.hpp>

bool c_base_player::get_bounding_box( box_t &box ) {
    auto origin = ( HASH( this->get_client_class( )->network_name ) == HASH_CT( "CCSPlayer" ) ) && this->bone_cache( ) ? this->bone_cache( )->get_origin( ) : this->origin( );

    vector_2d flb, brt, blb, frt, frb, brb, blt, flt;
    float left, top, right, bottom;

    auto min = this->collideable( )->mins( ) + origin;
    auto max = this->collideable( )->maxs( ) + origin;

    vector_3d points[] = {
            vector_3d( min.x, min.y, min.z ),
            vector_3d( min.x, min.y, max.z ),
            vector_3d( min.x, max.y, min.z ),
            vector_3d( min.x, max.y, max.z ),
            vector_3d( max.x, min.y, min.z ),
            vector_3d( max.x, min.y, max.z ),
            vector_3d( max.x, max.y, min.z ),
            vector_3d( max.x, max.y, max.z ),
    };

    if ( !render::world_to_screen( points[ 3 ], flb ) || !render::world_to_screen( points[ 5 ], brt ) || !render::world_to_screen( points[ 0 ], blb ) || !render::world_to_screen( points[ 4 ], frt ) || !render::world_to_screen( points[ 2 ], frb ) || !render::world_to_screen( points[ 1 ], brb ) || !render::world_to_screen( points[ 6 ], blt ) || !render::world_to_screen( points[ 7 ], flt ) ) {
        return false;
    }

    vector_2d arr[] = {
            flb,
            brt,
            blb,
            frt,
            frb,
            brb,
            blt,
            flt };

    left = flb.x;
    top = flb.y;
    right = flb.x;
    bottom = flb.y;

    for ( auto i = 1; i < 8; i++ ) {
        if ( left > arr[ i ].x )
            left = arr[ i ].x;

        if ( bottom < arr[ i ].y )
            bottom = arr[ i ].y;

        if ( right < arr[ i ].x )
            right = arr[ i ].x;

        if ( top > arr[ i ].y )
            top = arr[ i ].y;
    }

    box.x = static_cast< int >( left );
    box.y = static_cast< int >( top );
    box.w = static_cast< int >( right - left );
    box.h = static_cast< int >( bottom - top );

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

void c_base_player::set_model_index( const int index ) {
    return utils::get_method< void( __thiscall * )( void *, int ) >( this, 75 )( this, index );
}

void c_base_player::update_clientside_animation( ) {
    return utils::get_method< void( __thiscall * )( void * ) >( this, 218 )( this );
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