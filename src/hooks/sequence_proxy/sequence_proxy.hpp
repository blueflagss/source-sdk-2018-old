#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace sequence_proxy
    {
        inline recv_var_proxy_fn original = { };

        void __cdecl hook( c_recv_proxy_data *data, void *player, void *a3 );

        void init( );
    }// namespace sequence_proxy
}// namespace hooks