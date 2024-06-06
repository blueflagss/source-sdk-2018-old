#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace end_scene
    {
        inline SafetyHookInline original = { };

        long __fastcall hook( REGISTERS, IDirect3DDevice9 *device );

        void init( );
    }// namespace end_scene
}// namespace hooks