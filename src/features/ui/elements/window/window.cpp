#include <blur/bBlur.hpp>
#include <elements.hpp>
#include <icons_fa.hpp>

void penumbra::window::animate( )
{
    this->window_height = this->cursor_position.y + this->titlebar_size.y + 10.5f;

    animations::lerp_to( HASH_CT( "form__lerp__" ) + HASH( this->name.c_str( ) ), this->window_height, 0.1f );
    animations::lerp_to( HASH_CT( "form__fade__" ) + HASH( this->name.c_str( ) ), this->is_resizing || this->is_resize_area_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "form__close__" ) + HASH( this->name.c_str( ) ), this->is_exit_hovered, 0.08f );
    animations::lerp_to( HASH_CT( "form__style__" ) + HASH( this->name.c_str( ) ), this->is_style_hovered, 0.08f );
}

void penumbra::window::paint( )
{
    if ( globals::fade_opacity[ this->get_main_window( ) ] < 0.05f )
        return;

    this->animate( );

    const auto animation = animations::get( HASH_CT( "form__fade__" ) + HASH( this->name.c_str( ) ) );
    const auto exit_animation = animations::get( HASH_CT( "form__close__" ) + HASH( this->name.c_str( ) ) );
    const auto style_animation = animations::get( HASH_CT( "form__style__" ) + HASH( this->name.c_str( ) ) );

    this->tab_height = 25.f;
    this->titlebar_size = glm::vec2{ this->size.x, this->tab_height };

    auto title_text_dimensions = render::get_text_size( fonts::montserrat_semibold, this->name );
    auto game_title_text_dimensions = render::get_text_size( fonts::montserrat_semibold, this->game_name );

    render::filled_rect( this->position.x - 2, this->position.y - 2, this->size.x + 4, this->size.y + 4, color{ 16, 16, 16, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 5.5f );
    render::filled_rect( this->position.x - 2, this->position.y - 2, this->size.x + 4, this->titlebar_size.y, color{ 22, 22, 22, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 5.5f, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight );

    render::string( fonts::visuals_segoe_ui, this->position.x + 8.0f, this->position.y - 2.5f + ( this->titlebar_size.y / 2 ) - title_text_dimensions.y / 2, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name );
    render::string( fonts::visuals_segoe_ui, this->position.x + 8.0f + ( this->size.x ) - game_title_text_dimensions.x, this->position.y - 2.5f + ( this->titlebar_size.y / 2 ) - game_title_text_dimensions.y / 2, color{ 100, 100, 100, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->game_name );

    if ( !this->tab_objects.empty( ) ) {
        for ( auto &object : this->tab_objects )
            object->paint( );

        auto lerped_value = animations::get( HASH( this->name.c_str( ) ), this->position.x );

        animations::lerp_to( HASH( this->name.c_str( ) ), this->current_page * ( this->size.x / this->tab_objects.size( ) ), 0.1f );


        render::gradient_rect( this->position.x, this->position.y + this->titlebar_size.y + this->tab_height - 2.0f, this->titlebar_size.x, this->titlebar_size.y / 2, color{ 12, 12, 12, 60 * globals::fade_opacity[ this->get_main_window( ) ] }, color{ 0, 0, 0, 0 }, true );       
        render::filled_rect( this->position.x + lerped_value.value, this->position.y + this->titlebar_size.y + this->tab_height - 2, this->size.x / this->tab_objects.size( ), 2.f, color{ globals::theme_accent, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 1.5f, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight );
    }

    if ( this->opened ) {
        auto icon_dimensions = render::get_text_size( fonts::font_awesome, ICON_FA_TIMES );
        auto exit_indicator_position = glm::vec2( this->position.x + this->size.x - icon_dimensions.x - 10.0f, this->position.y - 1.5f + 7.5f );
        auto exit_indicator_size = glm::vec2( icon_dimensions.x, icon_dimensions.y );

        render::string( fonts::font_awesome, exit_indicator_position, color{ 120, 120, 120, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 205, 205, 205, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, exit_animation.value ), ICON_FA_TIMES, false );
    }

    auto focused_object = get_focused< object >( );

    for ( auto &object : this->children_objects ) {
        if ( focused_object && focused_object == object.get( ) )
            continue;

        object->paint( );
    }

    render::filled_rect( this->position.x - 2, this->position.y + this->size.y - this->titlebar_size.y, this->size.x + 4, this->titlebar_size.y, color{ 22, 22, 22, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 5.5f, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight );


    if ( this->is_window_resizeable ) {
        auto icon_dimensions = render::get_text_size( fonts::font_awesome, ICON_FA_EXPAND_ALT );
        auto resize_indicator_position = glm::vec2( this->position + this->size - glm::vec2( 6.5f, 4.0f ) - icon_dimensions );
        auto resize_indicator_size = glm::vec2( 10.0f, 10.0f );

      /*  render::filled_rect( this->position + this->size - glm::vec2( 13.0f, 11.0f ) - icon_dimensions, icon_dimensions + glm::vec2( 13.0f, 11.0f ), color{ 35, 35, 35, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight );*/
        render::string( fonts::font_awesome, resize_indicator_position, color{ 65, 65, 65, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation.value ), ICON_FA_EXPAND_ALT, false );
    }

    if ( globals::tooltip ) {
        render::string( fonts::visuals_segoe_ui, this->position.x + 8.0f, this->position.y + this->size.y - 21.0f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation.value ), reinterpret_cast< penumbra::tooltip * >( globals::tooltip )->name, false );
    }

    render::rect( this->position.x - 1, this->position.y - 1, this->size.x + 2, this->size.y + 2, color{ 48, 48, 48, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 5.5f );
    render::rect( this->position.x - 2, this->position.y - 2, this->size.x + 4, this->size.y + 4, color{ 18, 18, 18, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 5.5f );

    if ( focused_object )
        focused_object->paint( );

}

void penumbra::window::input( )
{
    this->cursor_position = { 0.0f, 0.0f };

    auto icon_dimensions = render::get_text_size( fonts::font_awesome, ICON_FA_EXPAND_ALT );

    this->is_hovered = input::in_region( this->position, this->flags == WINDOW_INDICATOR ? this->size : this->titlebar_size );
    this->is_resize_area_hovered = input::in_region( this->position + this->size - glm::vec2( 7.0f, 5.0f ) - icon_dimensions, icon_dimensions + glm::vec2( 7.0f, 5.0f ) );
    this->is_hovered_in_rectangle = input::in_region( this->position, this->size );

    auto focused_object = get_focused< object >( );

    if ( !focused_object && this->opened ) {
        auto icon_dimensions = render::get_text_size( fonts::font_awesome, ICON_FA_TIMES );
        auto style_indicator_position = glm::vec2( this->position.x + this->size.x - icon_dimensions.x - 10.0f, this->position.y + 7.5f );
        auto style_indicator_size = glm::vec2( icon_dimensions.x, icon_dimensions.y );

        this->is_exit_hovered = input::in_region( style_indicator_position - glm::vec2( 10.0f, 15.0f ), style_indicator_size + glm::vec2( 18.0f, 15.0f ) );

        if ( this->is_exit_hovered && input::key_pressed( VK_LBUTTON ) )
            penumbra::settings_window->is_visible = false;
    }

    auto mouse_position = input::get_mouse_position( );

    if ( !this->should_disable_input && !focused_object ) {
        if ( !this->is_dragging && ( this->is_hovered && input::key_pressed( VK_LBUTTON ) ) ) {
            this->drag_delta.x = mouse_position.x - this->position.x;
            this->drag_delta.y = mouse_position.y - this->position.y;
            this->is_dragging = true;
        }

        if ( this->is_dragging && this->window_focused( ) ) {
            this->position.x = mouse_position.x - this->drag_delta.x;
            this->position.y = mouse_position.y - this->drag_delta.y;
        }

        this->position = {
                std::clamp< float >( this->position.x, 0.f, ::globals::ui::screen_size.x - this->size.x ),
                std::clamp< float >( this->position.y, 0.f, ::globals::ui::screen_size.y - this->size.y ) 
        };
    }

    if ( this->is_window_resizeable && this->window_focused( ) ) {
        if ( !this->is_resizing && ( this->is_resize_area_hovered && input::key_pressed( VK_LBUTTON ) ) ) {
            this->resize_delta.x = mouse_position.x - this->size.x;
            this->resize_delta.y = mouse_position.y - this->size.y;
            this->is_resizing = true;
        }

        if ( this->is_resizing ) {
            this->size.x = mouse_position.x - this->resize_delta.x;
            this->size.y = mouse_position.y - this->resize_delta.y;
        }

        if ( this->is_resizing && !input::key_down( VK_LBUTTON ) )
            this->is_resizing = false;
    }

    auto animation = animations::get( HASH_CT( "form__lerp__" ) + HASH( this->name.c_str( ) ), this->titlebar_size.y );

    if ( this->flags == WINDOW_INDICATOR )
        this->size.y = std::max< float >( 0.0f, animation.value );

    else {
        this->size = {
                std::clamp< float >( this->size.x, this->_original_size.x, ::globals::ui::screen_size.x - this->position.x ),
                std::clamp< float >( this->size.y, this->_original_size.y, ::globals::ui::screen_size.y - this->position.y )
        };
    }

    if ( this->is_dragging && !input::key_down( VK_LBUTTON ) )
        this->is_dragging = false;

    if ( input::in_region( this->position, this->size ) && input::key_pressed( VK_LBUTTON ) )
        this->last_click_time = GetTickCount( );

    if ( !this->tab_objects.empty( ) ) {
        auto focused_object = get_focused< penumbra::object >( );

        for ( int i = { }; i < this->tab_objects.size( ); i++ ) {
            auto &object = this->tab_objects[ i ];

            object->position = glm::vec2(
                    std::roundf( this->position.x + 2 + i * ( this->size.x / this->tab_objects.size( ) ) - 2.5f ),
                    this->position.y + this->titlebar_size.y - 2 );

            object->size = glm::vec2(
                    std::roundf( ( this->size.x / this->tab_objects.size( ) ) + 0.5f ),
                    this->tab_height );

            if ( !focused_object && this->window_focused( ) ) {
                object->is_hovered = input::in_region( object->position, object->size );

                if ( object->is_hovered && input::key_pressed( VK_LBUTTON ) )
                    this->current_page = i;
            }

            object->is_selected = this->current_page == i;
            object->input( );
        }
    }

    for ( auto &object : this->children_objects ) {
        object->position = {
                this->position.x + 13.f,
                this->position.y + this->titlebar_size.y + this->cursor_position.y + 8.0f };

        this->handle_object( object );

        if ( focused_object && focused_object != object.get( ) )
            continue;

        object->input( );
    }
}

void penumbra::window::handle_object( const std::shared_ptr< penumbra::object > &object ) {
    switch ( object->element_type ) {
        case TYPE_CHECKBOX: {
            object->size = glm::vec2{ 14.0f, 14.0f };

            this->cursor_position.y += object->size.y + 7.5f;
        } break;
        case TYPE_LABEL: {
            this->cursor_position.y += 23.5f;
        } break;
        case TYPE_BUTTON: {
            object->size = glm::vec2{ this->size.x - 20.0f, 20.0f };

            this->cursor_position.y += object->size.y + 7.0f;
        } break;
        case TYPE_COMBOBOX: {
            object->size = glm::vec2{ this->size.x - 20.0f, 20.0f };

            this->cursor_position += COMBOBOX_OFFSET + glm::vec2{ 0.0f, object->size.y + 6.0f };
        } break;
        case TYPE_LISTBOX: {
            auto list = reinterpret_cast< listbox * >( object.get( ) );

            object->size = glm::vec2{ this->size.x - 20.0f, list->height };

            this->cursor_position += LISTBOX_OFFSET + glm::vec2{ 0.0f, object->size.y + 8.0f };
        } break;
        case TYPE_TEXTBOX: {
            object->size = glm::vec2{ this->size.x - 20.0f, 16.0f };

            this->cursor_position += TEXTBOX_OFFSET + glm::vec2{ 0.0f, object->size.y + 8.0f };
        } break;
        case TYPE_HOTKEY: {
            object->size = glm::vec2{ 159.0f, 12.0f };

            this->cursor_position += HOTKEY_OFFSET + glm::vec2{ 0.0f, object->size.y + 4.0f };
        } break;
        case TYPE_MULTI_COMBOBOX: {
            object->size = glm::vec2{ this->size.x - 20.0f, 20.0f };

            this->cursor_position += COMBOBOX_OFFSET + glm::vec2{ 0.0f, object->size.y + 6.0f };
        } break;
        case TYPE_SLIDER: {
            object->size = glm::vec2{ this->size.x - 20.0f, 5.5f };

            this->cursor_position += SLIDER_OFFSET + glm::vec2{ 0.0f, object->size.y + 7.0f };
        } break;
        case TYPE_INDICATOR: {
            object->size = glm::vec2{ this->size.x - 20.0f, 5.0f };

            this->cursor_position += INDICATOR_OFFSET + glm::vec2{ 0.0f, object->size.y + 11.0f };
        } break;
    }
}