#include "resolver.hpp"

void resolver::update( lag_record *previous, lag_record *record ) {

}

lag_record *resolver::find_ideal_record( aim_player *data ) {
    auto &log = g_animations.player_log[ data->entity->index() ];

    if ( !log.player )
        return nullptr;

    lag_record *first_valid, *current;

    if ( log.lag_records.empty( ) )
        return nullptr;

    first_valid = nullptr;

    // iterate records.
    for ( auto &it : log.lag_records ) {
        if ( !it ) continue;

        if ( it->dormant || !it->is_valid( ) )
            continue;

        // get current record.
        current = it;

        // first record that was valid, store it for later.
        if ( !first_valid )
            first_valid = current;

        // try to find a record with a shot, lby update, walking or no anti-aim.
        if ( it->mode == resolve_mode::walk || it->mode == resolve_mode::none )
            return current;
    }

    // none found above, return the first valid record if possible.
    return ( first_valid ) ? first_valid : find_last_record(data);
}

lag_record *resolver::find_last_record( aim_player *data ) {
    auto &log = g_animations.player_log[ data->entity->index( ) ];

    if ( !log.player )
        return nullptr;

    lag_record *current;

    if ( log.lag_records.empty( ) )
        return nullptr;

    // iterate records in reverse.
    for ( int i = static_cast< int >( log.lag_records.size( ) ) - 1; i >= 0; i-- ) {
        auto &record = log.lag_records[ i ];

        if ( !record ) continue;

        current = record;

        // if this record is valid.
        // we are done since we iterated in reverse.
        if ( current->is_valid( ) || !current->dormant )
            return current;
    }

    return nullptr;
}