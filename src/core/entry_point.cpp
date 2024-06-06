#include <core/config.hpp>
#include <core/hooks.hpp>
#include <features/features.hpp>
#include <features/visuals/chams.hpp>
#include <utils/logging/logging.hpp>
#include <globals.hpp>

HMODULE handle = nullptr;

int __stdcall cheat_main( void *loader_data ) {
    const auto start = std::chrono::high_resolution_clock::now( );

    while ( !GetModuleHandleA( "serverbrowser.dll" ) )
        std::this_thread::sleep_for( std::chrono::milliseconds( 400 ) );

    g_logging.init( );

    const auto end = std::chrono::high_resolution_clock::now( );
    const auto duration = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count( );

    g_interfaces.init( );
    g_config.init( );
    g_netvars.init( );

    int width, height;
    g_interfaces.engine_client->get_screen_size( width, height );

    globals::ui::screen_size = {
            static_cast< float >( width ),
            static_cast< float >( height )
    };

    g_chams.init( );
    g_menu.init( );
    g_hooks.init( );
    g_misc.unlock_hidden_cvars( );
    g_notify.add( "Loaded successfully" );

#ifdef _DEBUG
    spdlog::info( "cheat: initialized in {}ms.", duration );

    while ( !GetAsyncKeyState( VK_END ) )
        std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

    FreeLibraryAndExitThread( static_cast< HMODULE >( loader_data ), 0 );
#endif

    return 1;
}

int __stdcall DllMain( HINSTANCE module, unsigned long reason, void *loader_data ) {
    DisableThreadLibraryCalls( module );
    handle = module;

    switch ( reason ) {
        case DLL_PROCESS_ATTACH: {
            auto thread = CreateThread( 0, 0, reinterpret_cast< LPTHREAD_START_ROUTINE >( cheat_main ), module, 0, nullptr );

            if ( !thread )
                return 0;

            CloseHandle( thread );
        } break;
#ifdef _DEBUG
        case DLL_PROCESS_DETACH: {
            g_hooks.remove( );
            g_logging.remove( );
        } break;
#endif
    }

    return 1;
}