#include <elements.hpp>

void penumbra::label::paint( ) {
    render::string( fonts::montserrat, this->position.x, this->position.y - 1.5f, color{ 200, 200, 200, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->name );

    for ( auto &object : this->children_objects )
        object->paint( );
}

void penumbra::label::input( ) {
    auto dim = render::get_text_size( fonts::montserrat, this->name );
    auto parent_window = this->get_parent( );

    for ( auto &object : this->children_objects ) {
        object->position = {
                this->position.x + parent_window->size.x - object->size.x - 8.0f,
                this->position.y + object->size.y / 2
        };

        object->input( );
    }
}