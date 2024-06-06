#include "event_handler.hpp"
#include <features/ui/notifications/notifications.hpp>
#include <features/visuals/visuals.hpp>

event_handler::event_handler( ) {
    g_interfaces.event_manager->add_listener( this, "player_hurt", false );
    g_interfaces.event_manager->add_listener( this, "player_death", false );
    g_interfaces.event_manager->add_listener( this, "bullet_impact", false );
}

event_handler::~event_handler( ) {
    g_interfaces.event_manager->remove_listener( this );
}

void event_handler::fire_game_event( event_t *event ) {
    if ( !event )
        return;

    if ( HASH( event->get_name( ) ) == HASH_CT( "player_hurt" ) ) {
        auto attacker = g_interfaces.entity_list->get_client_entity< c_cs_player * >( g_interfaces.engine_client->get_player_for_user_id( event->get_int( "attacker" ) ) );
        auto victim_player = g_interfaces.entity_list->get_client_entity< c_cs_player * >( g_interfaces.engine_client->get_player_for_user_id( event->get_int( "userid" ) ) );

        if ( victim_player && globals::local_player == attacker ) {
            if ( g_vars.misc_hitmarker.value )
                g_visuals.hitmarker_fraction = 1.0f;

            if ( g_vars.misc_events_log_damage.value ) {
                auto damage = event->get_int( "dmg_health" );
                auto health = event->get_int( "health" );

                player_info_t player_info;

                if ( g_interfaces.engine_client->get_player_info( victim_player->index( ), &player_info ) )
                    g_notify.add( fmt::format( "Hit {} for {} ({} hp remaining)", player_info.name, damage, health ) );
            }
        }
    }
}

int event_handler::get_event_debug_id( ) {
    return 42;
}