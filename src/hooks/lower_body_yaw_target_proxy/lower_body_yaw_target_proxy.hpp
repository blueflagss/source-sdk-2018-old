#pragma once
#include <globals.hpp>

namespace hooks
{
    namespace lower_body_yaw_target_proxy
    {
        inline recv_var_proxy_fn original = { };

        void __cdecl hook( const c_recv_proxy_data *data, void *player, void *a3 );

        void init( );
    }// namespace lower_body_yaw_target_proxy
}// namespace hooks