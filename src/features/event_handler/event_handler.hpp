#pragma once
#include <globals.hpp>

class event_handler : public c_event_listener {
public:
    event_handler( );
    ~event_handler( );

    virtual void fire_game_event( event_t *event );
    int get_event_debug_id( );
};