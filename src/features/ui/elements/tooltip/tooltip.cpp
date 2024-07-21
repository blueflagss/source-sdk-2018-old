#include <elements.hpp>

void penumbra::tooltip::paint( ) {
    globals::tooltip = nullptr;

    const auto parent_window = this->get_parent< object >( )->get_parent( );

    if ( this->get_parent( )->is_hovered )
        globals::tooltip = this;
}

void penumbra::tooltip::input( ) {
    this->position = input::mouse_position + vector_2d( 10, 10 );
}
