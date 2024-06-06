#include "utils.hpp"

bool utils::is_point_visible( vector_3d pos_to_trace, c_base_entity *entity, int *hit_group ) {
    //c_game_trace trace = { };
    //c_trace_filter_hitscan filter = { };
    //filter.skip = globals::local_player;

    //ray_t ray;
    //ray.init( globals::local_player->eye_position( ), pos_to_trace );

    //g_interfaces.engine_trace->trace_ray( ray, ( mask_shot | contents_grate ), &filter, &trace );

    //if ( ( trace.entity && trace.entity == entity ) || trace.fraction >= 0.97f ) {
    //    if ( hit_group )
    //        *hit_group = trace.hit_group;

    //    return true;
    //}

    return false;
}

std::string utils::convert_utf8( wchar_t *text ) {
    auto text_wstring = std::wstring( text );
    auto final_text_string = std::string( text_wstring.begin( ), text_wstring.end( ) );

    return final_text_string;
}

std::vector< std::string > utils::split_str( std::string str, const char separator ) {
    std::vector< std::string > output;
    std::string substring;
    std::istringstream stream{ str };

    while ( std::getline( stream, substring, separator ) )
        output.push_back( substring );

    return output;
}