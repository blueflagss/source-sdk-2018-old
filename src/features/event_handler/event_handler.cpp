#include "event_handler.hpp"
#include <features/ui/notifications/notifications.hpp>
#include <features/visuals/visuals.hpp>
#include <features/sound_handler/sound_handler.hpp>
#include <features/grenade/grenade_warning.hpp>
#include <features/shot_manager/shot_manager.hpp>

event_handler::event_handler( ) {
    g_interfaces.event_manager->add_listener( this, _xs( "player_hurt" ), false );
    g_interfaces.event_manager->add_listener( this, _xs( "player_death" ), false );
    g_interfaces.event_manager->add_listener( this, _xs( "bullet_impact" ), false );
    g_interfaces.event_manager->add_listener( this, _xs( "round_start" ), false );
    g_interfaces.event_manager->add_listener( this, _xs( "weapon_fire" ), false );
    g_interfaces.event_manager->add_listener( this, _xs( "grenade_thrown" ), false );
}

event_handler::~event_handler( ) {
    g_interfaces.event_manager->remove_listener( this );
}

void event_handler::fire_game_event( event_t *event ) {
    if ( !globals::local_player || !event )
        return;

    switch ( HASH( event->get_name( ) ) ) {
        case HASH_CT( "grenade_thrown" ): {
            auto userid = g_interfaces.engine_client->get_player_for_user_id( event->get_int( _xs( "userid" ) ) );

            g_nade_events.clear( );
            g_nade_events.push_back( nade_event_t( userid, g_interfaces.global_vars->curtime ) );
        } break;
        case HASH_CT( "player_hurt" ): {
            g_shot_manager.on_hurt( event );
        } break;
        case HASH_CT( "bullet_impact" ): {
            g_shot_manager.on_impact( event );
        } break;
        case HASH_CT( "weapon_fire" ): {
            g_shot_manager.on_fire( event );
        } break;
        case HASH_CT( "round_start" ): {
            g_shot_manager.on_round_start( event );
            g_sound_handler.on_round_start( event );
        } break;;
    }
}

int event_handler::get_event_debug_id( ) {
    return 42;
}