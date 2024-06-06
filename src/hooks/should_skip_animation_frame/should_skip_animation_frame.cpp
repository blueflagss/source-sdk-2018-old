#include "should_skip_animation_frame.hpp"

bool __fastcall hooks::should_skip_animation_frame::hook( REGISTERS ) {
    return false;
}

void hooks::should_skip_animation_frame::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "57 8B F9 8B 07 8B ? ? ? ? ? FF D0 84 C0 75 02" ) ).get< void * >( ),
                                          should_skip_animation_frame::hook );
}