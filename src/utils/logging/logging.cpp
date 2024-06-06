#include "logging.hpp"

void logging::init( ) {
#ifdef _DEBUG
    FILE *stream = nullptr;

    if ( !AllocConsole( ) )
        return;

    auto hwnd = GetConsoleWindow( );

    SetConsoleTitleA( "csgo_legacy - debug" );

    if ( hwnd ) {
        freopen_s( reinterpret_cast< FILE ** >( stdout ), "CONOUT$", "w", stdout );
        SetWindowLong( hwnd, GWL_EXSTYLE, GetWindowLong( hwnd, GWL_EXSTYLE ) | WS_EX_LAYERED );
        SetLayeredWindowAttributes( hwnd, 0, 200, LWA_ALPHA );

        spdlog::info( "debug: console allocated." );
    }
#endif
}

void logging::remove( ) {
#ifdef _DEBUG
    fclose( reinterpret_cast< FILE * >( stdout ) );
    FreeConsole( );
    PostMessageW( GetConsoleWindow( ), WM_CLOSE, 0, 0 );
#endif
}