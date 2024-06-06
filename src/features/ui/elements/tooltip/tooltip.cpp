#include "tooltip.hpp"

void penumbra::tooltip::paint( ) {
    if ( this->get_parent( )->is_hovered ) {
        const auto text_dimensions = render::get_text_size( fonts::visuals_segoe_ui, this->name );
        const auto size = vector_2d( text_dimensions.x + 5, text_dimensions.y + 1.5f );

        render::filled_rect( this->position.x, this->position.y + ( size.y / 2 ) - ( text_dimensions.y / 2 ), size.x, size.y, color{ 10, 10, 10, 200 * globals::fade_opacity[ this->get_main_window< object >( ) ] }, 0.0f );
        render::string( fonts::visuals_segoe_ui, this->position.x + ( size.x / 2 ) - ( text_dimensions.x / 2 ), this->position.y + ( size.y / 2 ) - ( text_dimensions.y / 2 ) - 0.5f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window< object >( ) ] }, this->name, false );
    }
}

void penumbra::tooltip::input( ) {
    this->position = input::mouse_position + vector_2d( 10, 10 );
}
