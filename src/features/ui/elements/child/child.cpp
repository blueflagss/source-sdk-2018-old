#include <elements.hpp>
#include <icons_fa.hpp>

void penumbra::child::animate( ) {
    animations::lerp_to( HASH_CT( "child__lerp__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), this->window_height, 0.1f );
    animations::lerp_to( HASH_CT( "child__resize__icon__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), this->in_collapse_region, 0.1f );
}

void penumbra::child::paint( ) {
    this->animate( );

    const auto animation = animations::get( HASH_CT( "child__resize__icon__" ) + HASH( ( this->name + this->element_id ).c_str( ) ) );

    this->titlebar_size = glm::vec2{ this->size.x, 25.f };
    
    render::filled_rect( this->position, this->size, color{ 23, 23, 23, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f );
    render::scissor_rect( this->position.x, this->position.y, this->size.x, this->size.y, [ & ] {
        auto focused_object = get_focused< object >( );

        for ( auto &object : this->children_objects ) {
            if ( focused_object && focused_object == object.get( ) )
                continue;

            object->paint( );
        }
    } );

    auto text_dimensions = render::get_text_size( fonts::visuals_segoe_ui, this->name );

    render::rect( this->position.x - 1, this->position.y - 1, this->size.x + 2, this->size.y + 2, color{ 9, 9, 9, 50 *  globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f );
    render::rect( this->position, this->size, color{ 100, 100, 100, 100 * globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f );
    render::string( fonts::visuals_segoe_ui, this->position.x + 9, this->position.y - 8.5f, color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name, true );

    this->icon = this->is_collapsed ? "+" : "-";
    this->icon_dimensions = render::get_text_size( fonts::montserrat, this->icon );

    render::string( fonts::visuals_segoe_ui, this->position.x + text_dimensions.x + 15.0f - this->icon_dimensions.x / 2, this->position.y - 8.5f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation.value ), this->icon, false, true );
}

void penumbra::child::input( ) {
    this->cursor_position = glm::vec2{ 0.f, 0.f };

    auto focused_object = get_focused< penumbra::object >( );
    auto text_dimensions = render::get_text_size( fonts::visuals_segoe_ui, this->name );

    if ( !focused_object ) {
        this->in_collapse_region = input::in_region( this->position.x + text_dimensions.x + 15.0f - this->icon_dimensions.x / 2, this->position.y - 8.5f, this->icon_dimensions.x, this->icon_dimensions.y );

        if ( this->in_collapse_region && input::key_pressed( VK_LBUTTON ) )
            this->is_collapsed = !this->is_collapsed;
    }

    for ( auto &object : this->children_objects ) {
        object->position = {
                this->position.x + 10.f,
                this->position.y + this->cursor_position.y + 13.5f
        };

        this->handle_object( object );

        if ( focused_object && focused_object != object.get( ) )
            continue;

        object->input( );
    }

    if ( focused_object && ( focused_object->element_type == TYPE_COLORPICKER || focused_object->element_type == TYPE_HOTKEY ) )
        focused_object->input( );
}

void penumbra::child::handle_object( const std::shared_ptr< penumbra::object > &object )
{
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
            object->size = glm::vec2{ this->size.x - 20.0f, 20.0f };

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
