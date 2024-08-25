#include "update_clientside_animations.hpp"
#include <features/animations/animation_sync.hpp>
#include <features/bone_setup/bone_setup.hpp>

void __fastcall hooks::update_clientside_animation::hook( REGISTERS ) {
    const auto player = reinterpret_cast< c_cs_player * >( ecx );

    if ( !player || player == globals::local_player )
        return original.fastcall< void >( REGISTERS_OUT );
    
    if ( globals::allow_animations[ player->index( ) ] )
        original.fastcall< void >( REGISTERS_OUT );
}

void __stdcall hooks::update_clientside_animations::hook( ) {
    return original.stdcall< void >( );
}

void hooks::update_clientside_animation::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74" ) ).get< void * >( ),
                                          update_clientside_animation::hook );
}

void hooks::update_clientside_animations::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 50 10" ) ).add( 0x1 ).rel32( ).get< void* >( ),
                                          update_clientside_animations::hook );
}