#include <elements.hpp>

void penumbra::checkbox::animate( ) {
    animations::lerp_to( HASH_CT( "checkbox__hovered__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), this->is_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "checkbox__fade__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), *this->enabled, 0.1f );
}

void penumbra::checkbox::paint( ) {
    this->animate( );

    const auto animation = animations::get( HASH_CT( "checkbox__hovered__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_fade = animations::get( HASH_CT( "checkbox__fade__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), 0.0f );

    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position, this->size, color{ 47, 47, 47, 200 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 65, 65, 65, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 3.0f );
    render::filled_rect( this->position, this->size, color{ globals::theme_accent, 255 * globals::fade_opacity[ this->get_main_window( ) ] * animation_fade.value }, 3.0f );

    render::rect( this->position, this->size, color{ 9, 9, 9, 120 * globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );
    render::string( fonts::visuals_segoe_ui, this->position.x + this->size.x + 6.0f, this->position.y - 2.0f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name, false );

    for ( auto &object : this->children_objects )
        object->paint( );
}

void penumbra::checkbox::input( ) {
    auto dim = render::get_text_size( fonts::visuals_segoe_ui, this->name );
    auto parent_window = this->get_parent( );

    this->total_bounds = glm::vec2{ this->size.x + dim.x + 6.f, this->size.y };

    if ( !this->blocked( ) ) {
        this->is_hovered = input::in_region( this->position, this->total_bounds );

        if ( this->is_hovered && input::key_pressed( VK_LBUTTON ) )
            *this->enabled = !*this->enabled;
    }

    for ( auto &object : this->children_objects ) {
        if ( object->element_type == TYPE_HOTKEY ) {
            object->position = {
                    this->position.x + parent_window->size.x - object->size.x - 9.5f,
                    this->position.y
            };
        } else {
            object->position = {
                    this->position.x + parent_window->size.x - object->size.x - 20.5f,
                    this->position.y 
            };
        }

        object->input( );
    }
}