#include <elements.hpp>

void penumbra::tab::paint( ) {
    auto parent_window = this->get_parent< penumbra::window >( );

    auto text_dim = render::get_text_size( fonts::montserrat_semibold, this->name );
    auto icon_dim = render::get_text_size( fonts::font_awesome, this->icon );

    auto tab_text_position = glm::vec2{ this->position.x + ( this->size.x / 2 ) - text_dim.x / 2, this->position.y + 9.6f + ( this->size.y / 2 ) - text_dim.y / 2 };
    auto tab_icon_position = glm::vec2{ this->position.x + ( this->size.x / 2 ) - icon_dim.x / 2, this->position.y - 5.9f + ( this->size.y / 2 ) - icon_dim.y / 2 };

    const auto animation = animations::get( HASH_CT( "tab__" ) + HASH( this->name.c_str( ) ), 0.0f );

    animations::lerp_to( HASH_CT( "tab__" ) + HASH( this->name.c_str( ) ), this->is_selected || this->is_hovered, 0.1f );

    // Animate.
    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position.x, this->position.y, this->size.x, this->size.y, color{ 35, 35, 35, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color { 25, 25, 25, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ) );

    const auto animation_fade = animations::get( HASH_CT( "tab__fade__" ) + HASH( this->name.c_str( ) ), 0.0f );

    // Lerp animations.
    animations::lerp_to( HASH_CT( "tab__fade__" ) + HASH( this->name.c_str( ) ), this->is_selected, 0.1f );

    // Render fade & text.
    //render::gradient_rect( this->_pos.x, this->_pos.y, this->_size.x, this->_size.y, color{ 0, 0, 0, 0 }, color{ 0, 0, 0, 0 }.lerp( color{ globals::theme_accent, 40 *  globals::fade_opacity[ this->get_main_window( ) ] }, animation_fade.value ), true );
    render::string( fonts::font_awesome, tab_icon_position, color{ 150, 150, 150, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), this->icon, false );
    render::string( fonts::montserrat_main, tab_text_position, color{ 150, 150, 150, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), this->name );

    if ( !this->is_selected )
        return;

    render::scissor_rect( parent_window->position.x, parent_window->position.y + parent_window->titlebar_size.y, parent_window->size.x, parent_window->size.y - parent_window->titlebar_size.y, [ & ] {
        if ( this->num_columns ) {
            for ( int i = { }; i < this->num_columns; i++ ) {
                for ( int n = { }; n < this->_child_windows[ i ].size( ); n++ ) {
                    auto &object = this->_child_windows[ i ][ n ];

                    object->paint( );
                }
            }
        }
    } );
}

void penumbra::tab::input( ) {
    auto parent = this->get_parent< penumbra::window >( );

    if ( !this->is_selected )
        return;

    if ( this->num_columns ) {
        auto column_width = ( parent->size.x / this->num_columns ) - 7.0f;

        for ( int i = 0; i < this->num_columns; i++ ) {
            float total_cursor_offset = 0.0f;

            for ( int n = 0; n < this->_child_windows[ i ].size( ); n++ ) {
                auto &object = this->_child_windows[ i ][ n ];
               
                object->window_height = object->cursor_position.y + object->titlebar_size.y + 10.5f;

                auto animation = animations::get( HASH_CT( "child__lerp__" ) + HASH( ( object->name + object->element_id ).c_str( ) ), object->window_height );
                       
                if ( object->is_collapsed )
                    object->window_height = 30.0f;

                total_cursor_offset += object->window_height + 10.f;

                object->position = {
                        parent->position.x + 16.5f + ( column_width * i ),
                        parent->position.y + parent->tab_height + 32.f + total_cursor_offset - object->window_height
                };

                object->size = {
                        ( parent->size.x / this->num_columns ) - 15.f,
                        animation.value
                };

                object->input( );
            }
        }
    }
}