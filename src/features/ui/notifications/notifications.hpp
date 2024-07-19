#pragma once
#include <core/variables.hpp>
#include <features/ui/animations/animations.hpp>
#include <globals.hpp>

struct notification {
    notification( ) = default;

    notification( std::string text, ... ) {
        this->text = text;
        this->active = true;
        this->time = 9.0f;
        this->color = color::white( );
    }

    std::string text;
    bool active;
    float time;
    color color;
};

enum class notify_type : int {
    none = -1,
    shot,
    miss,
    error,
    info
};

class notifications {
public:
    std::deque< std::shared_ptr< notification > > active_notifications;

    template< typename... Args >
    void add( const notify_type &type, bool dont_paint, Args &&...args ) {
        auto console_text_color = color( 200, 200, 200 );
        auto console_text_tag_color = color( 0, 0, 0 );

        auto get_notify_tag = [ & ]( ) -> std::string {
            switch ( type ) {
                case notify_type::miss: {
                    console_text_tag_color = color( 255, 0, 13 );
                    console_text_color = color( 56, 56, 56 );

                    return _xs( "[miss] " );
                } break;
                case notify_type::shot: {
                    console_text_tag_color = color( 126, 41, 255 );
                    console_text_color = color( 200, 200, 200 );

                    return _xs( "[shot] " );
                } break;
                default: break;
            }

            return "";
        };

        g_interfaces.cvar->console_color_printf( g_vars.ui_theme.value, _xs( "[geekbar] " ) );

        if ( type != notify_type::none )
            g_interfaces.cvar->console_color_printf( console_text_tag_color, fmt::format( _xs( "{}" ), get_notify_tag( ) ).data( ) );

        const auto print_console_vardiac = [ & ]( auto &&arguments ) -> void 
        {
            g_interfaces.cvar->console_color_printf( console_text_color, std::forward< std::string >( arguments ).data( ) );
        };

        ( print_console_vardiac( args ), ... );
        g_interfaces.cvar->console_color_printf( color::white( ), "\n" );

        if ( !dont_paint )
            active_notifications.emplace_back( std::make_shared< notification >( std::forward< Args >( args )... ) );
    }

    void render( );
};// namespace events

inline notifications g_notify;