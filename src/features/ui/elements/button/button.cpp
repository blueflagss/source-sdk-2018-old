#include <elements.hpp>

void penumbra::button::animate( ) {
    animations::lerp_to( HASH_CT( "button__hovered__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), get_focused< object >( ) == this || this->is_hovered, 0.1f );
}

void penumbra::button::paint( ) {
    auto text_dimensions = render::get_text_size( fonts::montserrat, this->name );

    this->animate( );

    const auto animation = animations::get( HASH_CT( "button__hovered__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), 0.0f );

    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position, this->size, color{ 24, 24, 24, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 32, 32, 32, 200 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 2.0f );
    render::rect( this->position, this->size, color{ 9, 9, 9, 120 *  globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );
    render::string( fonts::montserrat_semibold, this->position.x + ( this->size.x / 2 ) - text_dimensions.x / 2, this->position.y + 3.5f, color{ 150, 150, 150, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 200 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), this->name );
}

void penumbra::button::input( ) {
    if ( this->blocked( ) )
        return;

    this->is_hovered = input::in_region( this->position, this->size );

    if ( this->is_hovered && input::key_pressed( VK_LBUTTON ) )
        this->callback( );
}
