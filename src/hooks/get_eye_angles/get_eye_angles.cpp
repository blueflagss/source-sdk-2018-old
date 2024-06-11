#include "get_eye_angles.hpp"

vector_3d &__fastcall hooks::get_eye_angles::hook( REGISTERS ) {
    const auto original = reinterpret_cast< decltype( &hooks::get_eye_angles::hook ) >( hooks::get_eye_angles::original.trampoline( ).address( ) );

    if ( !ecx || ecx != globals::local_player )
        return original( REGISTERS_OUT );

    static auto ret_to_thirdperson_pitch = signature::find( XOR( "client.dll" ), XOR( "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55" ) ).get< void * >( );
    static auto ret_to_thirdperson_yaw = signature::find( XOR( "client.dll" ), XOR( "F3 0F 10 55 ? 51 8B 8E" ) ).get< void * >( );

    if ( _ReturnAddress( ) == ret_to_thirdperson_pitch || _ReturnAddress( ) == ret_to_thirdperson_yaw )
        return globals::lby_updating ? globals::sent_angles : globals::local_angles;

    return original( REGISTERS_OUT );
}

void hooks::get_eye_angles::init( ) {
    original = safetyhook::create_inline( signature::find( XOR( "client.dll" ), XOR( "56 8B F1 85 F6 74 32" ) ).get< void * >( ),
                                          get_eye_angles::hook );
}