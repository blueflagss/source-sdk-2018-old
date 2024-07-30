#include "config.hpp"
#include <features/ui/input/input.hpp>
#include <features/ui/notifications/notifications.hpp>

#define CONFIG_ASSERT( x )                                               \
    if ( reinterpret_cast< void * >( option_address ) == nullptr ) {     \
        spdlog::error( "Failed to load config value \"{}\".", x->name ); \
        continue;                                                        \
    }

void config::impl::init( ) {
    char file_path[ MAX_PATH ];

    if ( SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_APPDATA, NULL, 0, file_path ) ) ) {
        this->folder_path = std::string( file_path ) + "\\penumbra\\";

        if ( !std::filesystem::exists( this->folder_path ) )
            std::filesystem::create_directory( this->folder_path );
    }

    std::string default_name = this->folder_path + "Default.json";

    if ( !std::filesystem::exists( default_name ) ) {
        if ( !g_config.save( "Default" ) )
            return;
    }
}

bool config::impl::get_hotkey( const config_int &virtual_key, int key_type ) {
    if ( hotkey_states.find( virtual_key.name ) == hotkey_states.end( ) )
        hotkey_states[ virtual_key.name ] = penumbra::input::key_down( virtual_key.value );

    switch ( key_type ) {
        case hotkey_type::hold: {
            hotkey_states[ virtual_key.name ] = penumbra::input::key_down( virtual_key.value );
        } break;
        case hotkey_type::toggled: {
            if ( hotkey_states.find( virtual_key.name ) == hotkey_states.end( ) )
                hotkey_states[ virtual_key.name ] = penumbra::input::key_down( virtual_key.value );

            else if ( penumbra::input::key_pressed( virtual_key.value ) )
                hotkey_states[ virtual_key.name ] = !hotkey_states[ virtual_key.name ];
        } break;
        case hotkey_type::on: {
            hotkey_states[ virtual_key.name ] = true;
        } break;
    }

    return hotkey_states[ virtual_key.name ];
}

bool config::impl::save( const std::string &name ) {
    nlohmann::json j;

    std::ofstream file{ this->folder_path + name + ".json" };

    if ( !file.is_open( ) )
        return false;

    for ( size_t i = 0; i < NUM_CONFIG_ITEMS; i++ ) {
        uintptr_t option_address = reinterpret_cast< std::uintptr_t >( &g_vars ) + ( i * sizeof( config_bool ) );

        switch ( *reinterpret_cast< int * >( option_address ) ) {
            case config_type_bool: {
                auto option = reinterpret_cast< config_bool * >( option_address );
                CONFIG_ASSERT( option );

                j[ option->name ] = option->value;
            } break;
            case config_type_int: {
                auto option = reinterpret_cast< config_int * >( option_address );
                CONFIG_ASSERT( option );

                j[ option->name ] = option->value;
            } break;
            case config_type_float: {
                auto option = reinterpret_cast< config_float * >( option_address );
                CONFIG_ASSERT( option );

                j[ option->name ] = option->value;
            } break;
            case config_type_double: {
                auto option = reinterpret_cast< config_double * >( option_address );
                CONFIG_ASSERT( option );

                j[ option->name ] = option->value;
            } break;
            case config_type_string: {
                auto option = reinterpret_cast< config_string * >( option_address );
                CONFIG_ASSERT( option );

                j[ option->name ] = option->value.c_str( );
            } break;
            case config_type_color: {
                auto option = reinterpret_cast< config_color * >( option_address );
                CONFIG_ASSERT( option );

                j[ option->name ][ 0 ] = option->value.r;
                j[ option->name ][ 1 ] = option->value.g;
                j[ option->name ][ 2 ] = option->value.b;
                j[ option->name ][ 3 ] = option->value.a;
            } break;
            case config_type_vector_3d: {
                auto option = reinterpret_cast< config_vector_3d * >( option_address );
                CONFIG_ASSERT( option );

                j[ option->name ][ 0 ] = option->value.x;
                j[ option->name ][ 1 ] = option->value.y;
                j[ option->name ][ 2 ] = option->value.z;
            } break;
            default: {
                spdlog::error( "Found config value with unknown type" );
                return false;
            } break;
        }
    }

    file << std::setw( 4 ) << j << std::endl;
    file.close( );

    g_notify.add( notify_type::info, false, fmt::format( "Saved config \"{}\"", name ) );

    return true;
}

bool config::impl::remove( const std::string &name ) {
    std::remove( std::string( this->folder_path + name + ".json" ).c_str( ) );

    g_notify.add( notify_type::info, false, fmt::format( "Removed config \"{}\"", name ) );

    return true;
}

bool config::impl::load( const std::string &name ) {
    std::ifstream file{ this->folder_path + name + ".json" };

    if ( !file.is_open( ) )
        return false;

    nlohmann::json j;

    file >> j;

    for ( size_t i = 0; i < NUM_CONFIG_ITEMS; i++ ) {
        uintptr_t option_address = reinterpret_cast< std::uintptr_t >( &g_vars ) + ( i * sizeof( config_bool ) );

        switch ( *reinterpret_cast< int * >( option_address ) ) {
            case config_type_bool: {
                auto option = reinterpret_cast< config_bool * >( option_address );
                CONFIG_ASSERT( option );

                option->value = j[ option->name ];
            } break;
            case config_type_int: {
                auto option = reinterpret_cast< config_int * >( option_address );
                CONFIG_ASSERT( option );

                option->value = j[ option->name ];
            } break;
            case config_type_float: {
                auto option = reinterpret_cast< config_float * >( option_address );
                CONFIG_ASSERT( option );

                option->value = j[ option->name ];
            } break;
            case config_type_double: {
                auto option = reinterpret_cast< config_double * >( option_address );
                CONFIG_ASSERT( option );

                option->value = j[ option->name ];
            } break;
            case config_type_string: {
                auto option = reinterpret_cast< config_string * >( option_address );
                CONFIG_ASSERT( option );

                option->value = j[ option->name ];
            } break;
            case config_type_color: {
                auto option = reinterpret_cast< config_color * >( option_address );
                CONFIG_ASSERT( option );

                option->value.r = j[ option->name ][ 0 ];
                option->value.g = j[ option->name ][ 1 ];
                option->value.b = j[ option->name ][ 2 ];
                option->value.a = j[ option->name ][ 3 ];
            } break;
            case config_type_vector_3d: {
                auto option = reinterpret_cast< config_vector_3d * >( option_address );
                CONFIG_ASSERT( option );

                option->value.x = j[ option->name ][ 0 ];
                option->value.y = j[ option->name ][ 1 ];
                option->value.z = j[ option->name ][ 2 ];
            } break;
            default: {
                spdlog::error( "Found config value with unknown type" );
                return false;
            } break;
        }
    }

    file.close( );

    g_notify.add( notify_type::info, false, fmt::format( "Loaded config \"{}\"", name ) );

    return true;
}