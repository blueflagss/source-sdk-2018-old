#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace check_for_sequence_change
    {
        inline SafetyHookInline original = { };

        void __fastcall hook( REGISTERS, void *hdr, int sequence, bool force_new_sequence, bool interpolate );

        void init( );
    }// namespace check_for_sequence_change
}// namespace hooks
