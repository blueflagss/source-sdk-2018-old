#include <elements.hpp>
#include <icons_fa.hpp>

void penumbra::hotkey::animate( ) {
    auto hotkey_dimensions = render::get_text_size( fonts::montserrat_main, this->_hotkey_text );
    auto center_object = glm::vec2{ this->position.x + ( this->size.x / 2 ) - hotkey_dimensions.x / 2, this->position.y + ( this->size.y / 2 ) - hotkey_dimensions.y / 2 };

    auto hotkey_rectangle_position = glm::vec2{ this->position.x - 9.0f - hotkey_dimensions.x, this->position.y - 2.0f };
    auto hotkey_rectangle_dimensions = glm::vec2{ hotkey_dimensions.x - 5.0f + 15.0f, hotkey_dimensions.y + 3.0f };

    animations::lerp_to( HASH_CT( "hotkey__hovered__" ) + HASH( this->name.c_str( ) ), this->is_hovered || get_focused< object >( ) == this, 0.1f );
    animations::lerp_to( HASH_CT( "hotkey__fade__" ) + HASH( this->name.c_str( ) ), ( this->focused_type == keybind_focus_type::FOCUS_BIND && get_focused< object >( ) == this ) || this->is_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "hotkey__lerp__" ) + HASH( this->name.c_str( ) ), hotkey_rectangle_dimensions.x, 0.15f );
    animations::lerp_to( HASH_CT( "selection__open__lerp__" ) + HASH( this->name.c_str( ) ), this->toggle_opened, 0.19f );
}

std::vector< std::string > toggle_list = {
        "Toggle",
        "Hold",
        "On" 
};

void penumbra::hotkey::paint( ) {
    this->animate( );

    const auto animation = animations::get( HASH_CT( "hotkey__hovered__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_fade = animations::get( HASH_CT( "hotkey__fade__" ) + HASH( this->name.c_str( ) ), 0.0f );

    auto get_key_name = [ ]( const int virtual_key ) -> std::string {
        char output[ 16 ] = { };

        switch ( virtual_key ) {
            case VK_LBUTTON:  return "LMOUSE";
            case VK_RBUTTON:  return "RMOUSE";
            case VK_MBUTTON:  return "MOUSE3";
            case VK_XBUTTON1: return "MOUSE4";
            case VK_XBUTTON2: return "MOUSE5";
            case 0:           return "NONE";
            default:          break;
        }

        GetKeyNameTextA( MapVirtualKeyA( virtual_key, MAPVK_VK_TO_VSC ) << sizeof( output ), output, sizeof( output ) );

        return std::string( output );
    };

    this->_hotkey_text = get_key_name( *this->_bind );

    auto hotkey_dimensions = render::get_text_size( fonts::visuals_segoe_ui, this->_hotkey_text );

    const auto animation_lerp = animations::get( HASH_CT( "hotkey__lerp__" ) + HASH( this->name.c_str( ) ), hotkey_dimensions.x );
    const auto animation_open_lerp = animations::get( HASH_CT( "selection__open__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );

    auto hotkey_rectangle_position = glm::vec2{ this->position.x - 11.0f - animation_lerp.value, this->position.y - 2.0f };
    auto hotkey_rectangle_dimensions = glm::vec2{ animation_lerp.value, hotkey_dimensions.y + 3.0f };

    render::filled_rect( hotkey_rectangle_position.x, hotkey_rectangle_position.y, animation_lerp.value, hotkey_rectangle_dimensions.y, color{ 32, 32, 32, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 24, 24, 24, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ), 3.0f );
    render::rect( hotkey_rectangle_position.x, hotkey_rectangle_position.y, animation_lerp.value, hotkey_rectangle_dimensions.y, color{ 60, 60, 60, 100 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 50, 50, 50, 100 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ), 2.0f );

    render::scissor_rect( hotkey_rectangle_position.x, hotkey_rectangle_position.y, animation_lerp.value, hotkey_rectangle_dimensions.y, [ & ] {
        render::string( fonts::visuals_segoe_ui, hotkey_rectangle_position.x + ( animation_lerp.value / 2 ) - hotkey_dimensions.x / 2, this->position.y - 2.0f, color{ 120, 120, 120, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ globals::theme_accent, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ), this->_hotkey_text );
    } );

    if ( this->focused_type == keybind_focus_type::FOCUS_SELECTION && animation_open_lerp.value > 0.08f ) {
        auto selection_rectangle_dimension = glm::vec2{ 80.0f, 107.0f };

        render::filled_rect( hotkey_rectangle_position.x, this->position.y - 2.0f, hotkey_rectangle_dimensions.x, 5 + ( toggle_list.size( ) * 20.0f ) * animation_open_lerp.value, color{ 24, 24, 24, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 3.0f );
        render::rect( hotkey_rectangle_position.x, this->position.y - 2.0f, hotkey_rectangle_dimensions.x, 5 + ( toggle_list.size( ) * 20.0f ) * animation_open_lerp.value, color{ 60, 60, 60, 100 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 50, 50, 50, 100 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ), 2.0f );
        
        render::scissor_rect( hotkey_rectangle_position.x, this->position.y - 2.0f, hotkey_rectangle_dimensions.x, 5 + ( toggle_list.size( ) * 20.0f ) * animation_open_lerp.value, [ & ] {
            for ( int i = 0; i < toggle_list.size( ); i++ ) {
                auto hovered_over_item = input::in_region( this->position.x - 23.0f - hotkey_dimensions.x, this->position.y - 2.0f + ( i * 20.0f ), selection_rectangle_dimension.x, 20.0f );

                const auto animation_fade = animations::get( HASH_CT( "selection__fade__" ) + HASH( toggle_list[ i ].c_str( ) ), 0.0f );

                animations::lerp_to( HASH_CT( "selection__fade__" ) + HASH( toggle_list[ i ].c_str( ) ), hovered_over_item, 0.2f );

                render::string( fonts::visuals_segoe_ui, this->position.x - 19 - hotkey_dimensions.x, this->position.y + ( i * 20.0f ), color{ 200, 200, 200, 255 * animation_open_lerp.value * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ globals::theme_accent, 255 * animation_open_lerp.value * globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ), toggle_list[ i ] );
            }
        } );
    }
}

void penumbra::hotkey::input( ) {
    auto parent = this->get_parent< checkbox >( );
    auto hotkey_dimensions = render::get_text_size( fonts::visuals_segoe_ui, this->_hotkey_text );

    const auto animation_lerp = animations::get( HASH_CT( "hotkey__lerp__" ) + HASH( this->name.c_str( ) ), hotkey_dimensions.x );

    auto hotkey_rectangle_position = glm::vec2{ this->position.x - 9.0f - animation_lerp.value, this->position.y - 2.0f };
    auto hotkey_rectangle_dimensions = glm::vec2{ std::clamp< float >( animation_lerp.value - 5.0f + 15.0f, 60.0f, 100.0f ), hotkey_dimensions.y + 5.0f };

    this->is_hovered = input::in_region( hotkey_rectangle_position.x, hotkey_rectangle_position.y, animation_lerp.value, hotkey_rectangle_dimensions.y );

    if ( !parent->blocked( ) ) {
        const auto is_focused = get_focused< object >( ) == this;

        if ( ( this->is_hovered && input::key_pressed( VK_RBUTTON ) ) ) {
            add_object_to_focus( this );

            this->focused_type = keybind_focus_type::FOCUS_SELECTION;
            this->toggle_opened = true;
        }

        if ( !is_focused && ( this->is_hovered && input::key_pressed( VK_LBUTTON ) ) ) {
            this->focused_type = keybind_focus_type::FOCUS_BIND;

            add_object_to_focus( this );

            this->_is_set = false;
            this->_old_set = false;
        }
    }

    const auto animation_open_lerp = animations::get( HASH_CT( "selection__open__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );

    if ( !this->toggle_opened && this->focused_type == keybind_focus_type::FOCUS_SELECTION && get_focused< object >( ) == this && animation_open_lerp.value < 0.12f ) {
        this->focused_type = keybind_focus_type::NONE;
        this->toggle_opened = false;
        remove_object_focus( );
    }

    if ( get_focused< object >( ) == this ) {
        switch ( this->focused_type ) {
            case keybind_focus_type::FOCUS_BIND: {
                if ( ( !this->_is_set && !this->_old_set ) && ( this->is_hovered && input::key_pressed( VK_LBUTTON ) ) )
                    this->_is_set = true;

                else if ( this->_is_set ) {
                    for ( int i = 0; i < 256; i++ ) {
                        if ( i != VK_LBUTTON ) {
                            /* reset if escape is clicked */ {
                                if ( i == VK_ESCAPE && input::key_pressed( i ) ) {
                                    *this->_bind = 0;
                                    this->_is_set = false;
                                    this->_old_set = true;
                                    remove_object_focus( );
                                    break;
                                }
                            }

                            /* handle keys */ {
                                if ( input::key_released( i ) ) {
                                    *this->_bind = i;
                                    this->_is_set = false;
                                    this->_old_set = true;
                                    remove_object_focus( );
                                    break;
                                }
                            }
                        }
                    }
                }

                else
                    this->_old_set = false;
            } break;
            case keybind_focus_type::FOCUS_SELECTION: {
                auto selection_rectangle_dimension = glm::vec2{ 80.0f, 100.0f };

                for ( int i = 0; i < toggle_list.size( ); i++ ) {
                    auto hovered_over_item = input::in_region( hotkey_rectangle_position.x, this->position.y - 2.0f + ( i * 20.0f ), selection_rectangle_dimension.x, 20.0f );

                    if ( hovered_over_item && input::key_pressed( VK_LBUTTON ) ) {
                        *this->_toggle = i;
                        this->toggle_opened = false;
                    }
                }

                auto in_selection_region = input::in_region( hotkey_rectangle_position.x, this->position.y - 2.0f, selection_rectangle_dimension.x, ( toggle_list.size( ) * 20.0f ) * animation_open_lerp.value );

                if ( !in_selection_region && input::key_pressed( VK_LBUTTON ) )
                    this->toggle_opened = false;
            } break;
        }
    }
}