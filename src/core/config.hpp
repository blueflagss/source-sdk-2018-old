#pragma once
#include <globals.hpp>
#include "variables.hpp"

enum hotkey_type : int {
    toggled = 0,
    hold,
    on
};

namespace config
{
    class impl {
    public:
        void init( );
        void load_skins( );
        void save_skins( );
        bool get_hotkey( const config_int &virtual_key, int key_type );
        bool save( const std::string &name );
        bool remove( const std::string &name );
        bool load( const std::string &name );

    private:
        std::unordered_map< std::string, bool > hotkey_states;

    public:
        std::string folder_path = { };

    };
}// namespace config

inline config::impl g_config = { };