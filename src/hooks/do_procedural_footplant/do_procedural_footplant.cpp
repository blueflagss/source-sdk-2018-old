#include "do_procedural_footplant.hpp"

void __fastcall hooks::do_procedural_footplant::hook( REGISTERS, matrix_3x4_aligned bone_to_world[ ], void *left_foot_chain, void *right_foot_chain, vector_3d pos[ ] ) {
    return;
}

void hooks::do_procedural_footplant::init( ) {
    original = safetyhook::create_inline( signature::find( _xs( "client.dll" ), _xs( "E8 ? ? ? ? F6 86 ? ? ? ? ? 0F 84" ) ).add( 0x1 ).rel32( ).get< void * >( ),
                                          do_procedural_footplant::hook );
}