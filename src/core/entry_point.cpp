#include <core/config.hpp>
#include <core/hooks.hpp>
#include <features/features.hpp>
#include <features/visuals/chams.hpp>
#include <globals.hpp>
#include <threadutils/semaphores.h>
#include <threadutils/shared_mutex.h>
#include <threadutils/threading.h>
#include <utils/logging/logging.hpp>

HMODULE handle = nullptr;
typedef int ( *ThreadIDFn )( void );
ThreadIDFn AllocateThreadID = nullptr;
ThreadIDFn FreeThreadID = nullptr;

static Semaphore dispatchSem;
static Semaphore waitSem;
static SharedMutex smtx;

template< typename T, T &Fn >
static void AllThreadsStub( void * ) {
    dispatchSem.Post( );
    smtx.rlock( );
    smtx.runlock( );
    Fn( );
}

//TODO: Build this into the threading library
template< typename T, T &Fn >
static void DispatchToAllThreads( void *data ) {
    smtx.wlock( );

    for ( size_t i = 0; i < Threading::numThreads; i++ )
        Threading::QueueJobRef( AllThreadsStub< T, Fn >, data );

    for ( size_t i = 0; i < Threading::numThreads; i++ )
        dispatchSem.Wait( );

    smtx.wunlock( );

    Threading::FinishQueue( false );
}

int __stdcall cheat_main( void *loader_data ) {
    const auto start = std::chrono::high_resolution_clock::now( );

    while ( !GetModuleHandleA( "serverbrowser.dll" ) )
        std::this_thread::sleep_for( std::chrono::milliseconds( 400 ) );

    AllocateThreadID = ( ThreadIDFn ) ( GetProcAddress( GetModuleHandleA( _xs( "tier0.dll" ) ), _xs( "AllocateThreadID" ) ) );
    FreeThreadID = ( ThreadIDFn ) ( GetProcAddress( GetModuleHandleA( _xs( "tier0.dll" ) ), _xs( "FreeThreadID" ) ) );

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
            static_cast< float >( height ) };

    Threading::InitThreads( );

#ifdef _DEBUG
    spdlog::info( "Spawned {} threads", Threading::numThreads );
#endif

    DispatchToAllThreads< ThreadIDFn, AllocateThreadID >( nullptr );
    Threading::FinishQueue( );

    g_chams.init( );
    g_menu.init( );
    g_hooks.init( );
    g_misc.unlock_hidden_cvars( );
    g_notify.add( notify_type::none, false, "Loaded successfully" );

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