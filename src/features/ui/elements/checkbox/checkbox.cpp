#include <elements.hpp>

void penumbra::checkbox::animate( ) {
    animations::lerp_to( HASH_CT( "checkbox__hovered__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), this->is_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "checkbox__fade__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), *this->enabled, 0.1f );
}

void penumbra::checkbox::render_checkmark( int x, int y, float scale, color color ) {
    render::line( { ( x + 7 * scale ) - 6, ( y + 15 * scale ) - 8 }, { ( x + 11 * scale ) - 6, ( y + 18 * scale ) - 8 }, color, 2.5f );
    render::line( { ( x + 10 * scale ) - 6.5, ( y + 18 * scale ) - 8 }, { ( x + 18 * scale ) - 6.5, ( y + 10 * scale ) - 8 }, color, 2.5f );
}

void penumbra::checkbox::paint( ) {
    this->animate( );

    const auto animation = animations::get( HASH_CT( "checkbox__hovered__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_fade = animations::get( HASH_CT( "checkbox__fade__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), 0.0f );

    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position, this->size, color{ 27, 27, 27, 200 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 45, 45, 45, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 3.0f );
    render::filled_rect( this->position, this->size, color{ 20, 20, 20, 0 }.lerp( color{ globals::theme_accent, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ), 3.0f );

    render_checkmark( this->position.x + 3.0f, this->position.y + 3.0f, 0.89f, color{ 0, 0, 0, 0 }.lerp( color{ 0, 0, 0, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ) );

    render::rect( this->position, this->size, color{ 9, 9, 9, 120 * globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );
    render::string( fonts::montserrat, this->position.x + this->size.x + 7.f, this->position.y, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name, false );

    for ( auto &object : this->children_objects )
        object->paint( );
}

void penumbra::checkbox::input( ) {
    auto dim = render::get_text_size( fonts::montserrat, this->name );
    auto parent_window = this->get_parent( );

    this->total_bounds = glm::vec2{ this->size.x + dim.x + 6.f, this->size.y };

    if ( !this->blocked( ) ) {
        this->is_hovered = input::in_region( this->position, this->total_bounds );

        if ( this->is_hovered && input::key_pressed( VK_LBUTTON ) )
            *this->enabled = !*this->enabled;
    }

    for ( auto &object : this->children_objects ) {
        object->position = {
                this->position.x + parent_window->size.x - object->size.x - 7.5f,
                this->position.y + 1.0f 
        };

        object->input( );
    }
}