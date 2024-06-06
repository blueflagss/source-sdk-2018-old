#pragma once
#include <globals.hpp>

namespace hooks
{
    class impl {
    public:
        void init( );
        void remove( );
    };
}// namespace hooks

inline hooks::impl g_hooks = { };