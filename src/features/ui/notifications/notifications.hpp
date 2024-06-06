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

class notifications {
public:
    std::deque< std::shared_ptr< notification > > active_notifications;

    template< typename... Args >
    void add( Args &&...args ) {
        auto print_args = [ this ]( auto &&arg ) {
            g_interfaces.cvar->console_color_printf( color::white( ), std::forward< std::string >( arg ).c_str( ) );
        };

        g_interfaces.cvar->console_color_printf( g_vars.ui_theme.value, "nigger >> " );
        ( print_args( args ), ... );
        g_interfaces.cvar->console_color_printf( color::white( ), "\n" );

        /* add to list. */
        active_notifications.emplace_front( std::make_shared< notification >( std::forward< Args >( args )... ) );
    }

    void render( );
};// namespace events

inline notifications g_notify;