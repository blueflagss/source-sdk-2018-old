#include "cl_fire_events.hpp"

void __cdecl hooks::cl_fire_events::hook( ) {
    auto ei = g_interfaces.client_state->events( );

    client_event_t *next = nullptr;

    if ( !ei )
        return original.call< void >( );

    do {
        next = *reinterpret_cast< client_event_t ** >( reinterpret_cast< uintptr_t >( ei ) + 0x38 );

        if ( !ei->client_class ) continue;

        auto create_event_fn = ei->client_class->create_event_fn;

        if ( !create_event_fn ) {
            continue;
        }

        void *create_event = create_event_fn( );

        if ( !create_event ) {
            continue;
        }

        if ( ei->class_id == 169 )
            ei->fire_delay = 0.0f;

        ei = next;
    } while ( next != nullptr );

    return original.call< void >( );
}

void hooks::cl_fire_events::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "engine.dll" ), XOR( "55 8B EC 83 EC 08 53 8B 1D ? ? ? ? 56 57 83 BB ? ? ? ? ? 74" ) ).get< void * >( ),
                                          cl_fire_events::hook );
}