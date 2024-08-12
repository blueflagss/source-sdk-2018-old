#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace post_data_update
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, int update_type );

        void init( );
    }// namespace post_data_update
}// namespace hooks