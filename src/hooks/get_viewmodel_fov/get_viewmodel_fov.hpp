#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace get_viewmodel_fov
    {
        inline SafetyHookInline original = { };

        float __fastcall hook( REGISTERS );

        void init( );
    }// namespace get_viewmodel_fov
}// namespace hooks