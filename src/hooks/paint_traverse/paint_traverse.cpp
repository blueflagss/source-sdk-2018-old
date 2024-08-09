#include "paint_traverse.hpp"
#include <features/ui/window_setup.hpp>

void __fastcall hooks::paint_traverse::hook( REGISTERS, unsigned int panel, bool force_repaint, bool allow_force ) {
    const auto name = HASH( g_interfaces.panel->get_name( panel ) );

    switch ( name ) {
        case HASH_CT( "CompletionList" ):
        case HASH_CT( "GameConsole" ): {
            globals::is_console_toggled = true;
        } break;
    }

    /* call original. */
    original.fastcall< void >( REGISTERS_OUT, panel, force_repaint, allow_force );

    switch ( name ) {
        case HASH_CT( "CompletionList" ):
        case HASH_CT( "GameConsole" ): {
            globals::is_console_toggled = false;
        } break;
        case HASH_CT( "MatSystemTopPanel" ): {
        } break;
        case HASH_CT( "FocusOverlayPanel" ): {
            if ( !globals::local_player || !g_interfaces.engine_client->is_in_game( ) )
                break;

            /* update view matrix */
            v_matrix world_to_view, view_to_projection, world_to_pixels;
            c_view_setup view;

            static std::mutex data_mutex_yo{ };

            data_mutex_yo.lock( );
            g_interfaces.client->get_player_view( view );
            g_interfaces.render_view->get_matrices_for_view( view, &world_to_view, &view_to_projection, &globals::view_matrix, &world_to_pixels );
            data_mutex_yo.unlock( );
        } break;
    }
}

void hooks::paint_traverse::init( ) {
    original = safetyhook::create_inline( utils::get_method< void * >( g_interfaces.panel, 41 ),
                                          paint_traverse::hook );
}