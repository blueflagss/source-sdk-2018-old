#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace reset
    {
        inline SafetyHookInline original = { };

        long __stdcall hook( IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *presentation_parameters );

        void init( );
    }// namespace reset
}// namespace hooks