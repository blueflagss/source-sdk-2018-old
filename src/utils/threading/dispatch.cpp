#include "dispatch.hpp"

using namespace threading;

void dispatch_queue::spawn( ) {
    allocate_thread_id = allocate_thread_id_t( GetProcAddress( GetModuleHandleA( "tier0.dll" ), "AllocateThreadID" ) );
    free_thread_id = free_thread_id_t( GetProcAddress( GetModuleHandleA( "tier0.dll" ), "FreeThreadID" ) );

    const auto slots = min( std::thread::hardware_concurrency( ) - 1, calculate_used_threads( ) );

    if ( slots < 1 ) {
        MessageBoxA( nullptr, _xs( "unable to allocate threads" ), _xs( "fail" ),
                     MB_ICONERROR | MB_SETFOREGROUND );
        ExitProcess( 0 );
    }

    for ( auto i = 0u; i < slots; i++ )
        threads.emplace_back( &dispatch_queue::process, this, i );

    spdlog::info( "{} threads spawned and ready for dispatch", threads.size() );
}

void dispatch_queue::evaluate( std::vector< fn > &f ) {
    if ( f.empty( ) )
        return;

    std::unique_lock lock( queue_mtx );
    queue = &f;
    to_run = queue->size( );

    for ( ;; ) {
        if ( queue->empty( ) ) {
            auto val = 1;
            do {
                to_run.wait( val );
                val = to_run.load( std::memory_order_relaxed );
            } while ( val > 0 );
            break;
        }

        const auto current = std::move( queue->back( ) );
        queue->pop_back( );
        lock.unlock( );

        queue_cond.notify_all( );
        current( );

        to_run.fetch_sub( 1, std::memory_order_relaxed );
        lock.lock( );
    }

    queue = nullptr;
}

void dispatch_queue::decomission( ) {
    std::unique_lock lock( queue_mtx );

    if ( decomissioned )
        return;

    decomissioned = true;
    lock.unlock( );
    queue_cond.notify_all( );

    for ( auto &thread : threads )
        thread.join( );
}

void dispatch_queue::process( const size_t index ) {
    SetThreadAffinityMask( GetCurrentThread( ), 1 << index );

    allocate_thread_id( );

    for ( ;; ) {
        std::unique_lock lock( queue_mtx );
        queue_cond.wait( lock, [ this ] { return queue && !queue->empty( ) || decomissioned; } );

        if ( decomissioned )
            break;

        const auto current = std::move( queue->back( ) );
        queue->pop_back( );
        lock.unlock( );

        queue_cond.notify_all( );
        current( );

        --to_run;
        to_run.notify_one( );
    }

    free_thread_id( );
}

size_t dispatch_queue::calculate_used_threads( ) {
    static constexpr auto max_threads = 32;

    // really shitty hack, but works good so we don't steal threads from csgo

    auto highest_index = 1;
    while ( highest_index < 128 && g_addresses.tier0_allocated_thread_ids.get<uint8_t*>()[ highest_index ] )
        highest_index++;

    return std::clamp( max_threads - highest_index, 0, max_threads );
}