#include "miscellaneous.hpp"

void miscellaneous::unlock_hidden_cvars( ) {
    auto list = g_interfaces.cvar->get_commands( );

    if ( !list )
        return;

    for ( auto it = list; it != nullptr; it = it->next )
        it->flags &= ~( ( 1 << 4 ) | ( 1 << 1 ) ); /* FVAR_HIDDEN | FVAR_DEVELOPMENTONLY */
}