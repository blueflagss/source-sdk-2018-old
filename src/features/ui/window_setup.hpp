#pragma once

#include <array>
#include <features\ui\elements\elements.hpp>
#include <globals.hpp>
#include <memory>
#include <shared_mutex>

class menu {
public:
    std::shared_ptr< penumbra::window > add_window( const std::string &name, const std::string &game_name, bool *opened, glm::vec2 pos, glm::vec2 dim, form_flags flags ) {
        auto window = std::make_shared< penumbra::window >( name, game_name, opened, pos, dim, flags );

        penumbra::windows.emplace_back( window );

        return window;
    }

    void hide_ui( );
    void handle_hotkeys( );
    void on_screen_size_updated( int width, int height );
    void init_skins( );
    void init( );
    void refresh_config_items( );
    void display_watermark( );
    void render( );

public:
    std::shared_ptr< penumbra::window > main_window = nullptr;
    std::shared_ptr< penumbra::window > skinchanger = nullptr;

    bool is_initialized = false;
    bool is_opened( );

    std::shared_ptr< penumbra::combobox > config_list = nullptr; // g_vars.exploits_double_tap_limit.value
    std::shared_ptr< penumbra::indicator< int > > shift_indicator = nullptr;

    vector_2d main_window_dimensions{ 640.0f, 415.0f };
    vector_2d color_window_dimensions{ 364.0f, 440.0f };

    std::mutex mutex{ };
    std::deque< std::string > config_items{ };
    std::string config_name = "Default";

protected:
    bool is_visible = false;
    int config_selected = 0;
};

inline menu g_menu;