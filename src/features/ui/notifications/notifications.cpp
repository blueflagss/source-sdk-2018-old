#include "notifications.hpp"

namespace animation
{
    class notify_anim_ctx {
    public:
        notify_anim_ctx( ) {
        }

        notify_anim_ctx( float value, float last_value, std::shared_ptr< notification > notify ) {
            this->value = value;
            this->last_value = value;
            this->notify = notify;
        }

        float value;
        float last_value;
        std::shared_ptr< notification > notify;
    };

    inline std::unordered_map< std::shared_ptr< notification >, notify_anim_ctx > anim_map;

    inline notify_anim_ctx get( std::shared_ptr< notification > i, float default_value = 0.0f ) {
        auto query = anim_map.find( i );

        if ( query == anim_map.end( ) ) {
            anim_map.insert( std::pair< std::shared_ptr< notification >, notify_anim_ctx >( i, notify_anim_ctx( default_value, 0.f, i ) ) );
            query = anim_map.find( i );
        }

        return query->second;
    }

    inline void clear_map( ) {
        anim_map.clear( );
    }

    inline void lerp_to( std::shared_ptr< notification > notify, float destination, float additive, float min_max_clamp ) {
        const auto map = get( notify );
        auto value = anim_map.find( map.notify );

        if ( value->second.value != std::clamp< float >( destination, -min_max_clamp, min_max_clamp ) )
            value->second.value = std::clamp< float >( value->second.value + ( destination - value->second.value ) * ( ( 1.f / additive ) * ImGui::GetIO( ).DeltaTime ), -min_max_clamp, min_max_clamp );
    }
}// namespace animation

void notifications::render( ) {
    auto size = vector_2d( 400, 25 );
    auto position = vector_2d( -( size.x * 2 ), 0.0f );

    for ( size_t i = 0; i < active_notifications.size( ); ++i ) {
        auto notify = active_notifications[ i ];

        notify->time -= ImGui::GetIO( ).DeltaTime;

        if ( notify->time <= 0.1f ) {
            active_notifications.erase( active_notifications.begin( ) + i );
            continue;
        }

        if ( active_notifications.size( ) > 11 )
            active_notifications.pop_back( );
    }

    if ( active_notifications.empty( ) )
        return;

    for ( auto &notification : active_notifications ) {
        auto left = notification->time;
        bool expired = left <= 0.4f;

        animation::lerp_to( notification, !expired, 0.13f, 1000.f );

        auto fraction = animation::get( notification, 0.0f );
        auto pos = vector_2d( position.x + ( ( size.x * 2 ) * fraction.value ), position.y );

        /* back-ground */
        render::gradient_rect( pos, size, color{ 0, 0, 0, 0 * fraction.value }, color{ 30, 30, 30, 100 * fraction.value }, false );
        render::filled_rect( pos, size, color{ 30, 30, 30, 80 * fraction.value }, 3.0f );
        render::filled_rect( pos.x + size.x - 3.0f, pos.y, 3.0f, size.y, color{ g_vars.ui_theme.value, 255 * fraction.value }, 3.0f );

        /* text */
        render::string( fonts::montserrat_semibold, ( pos.x + 7 ), pos.y + 4, color{ 200, 200, 200, 255 * fraction.value }, notification->text );

        position.y += size.y;
    }
}