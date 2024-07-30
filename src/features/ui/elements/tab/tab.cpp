#include <elements.hpp>

void penumbra::tab::paint( ) {
    auto parent_window = this->get_parent< penumbra::window >( );

    auto text_dim = render::get_text_size( fonts::visuals_segoe_ui, this->name );
    auto icon_dim = render::get_text_size( fonts::font_awesome, this->icon );

    auto tab_text_position = glm::vec2{ this->position.x + icon_dim.x + 26.0f, this->position.y - 1.5f + ( this->size.y / 2 ) - text_dim.y / 2 };
    auto tab_icon_position = glm::vec2{ this->position.x + 17.0f, this->position.y + 1.0f + ( this->size.y / 2 ) - icon_dim.y / 2 };

    const auto animation = animations::get( HASH_CT( "tab__" ) + HASH( this->name.c_str( ) ), 0.0f );

    animations::lerp_to( HASH_CT( "tab__" ) + HASH( this->name.c_str( ) ), this->is_selected || this->is_hovered, 0.1f );

    // Animate.
    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position.x, this->position.y, this->size.x, this->size.y, color{ 35, 35, 35, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color { 25, 25, 25, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ) );

    const auto animation_fade = animations::get( HASH_CT( "tab__fade__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_scroll = animations::get( HASH_CT( "tab__scroll__" ) + HASH( this->name.c_str( ) ), 0.0f );

    // Lerp animations.
    animations::lerp_to( HASH_CT( "tab__scroll__" ) + HASH( this->name.c_str( ) ), this->scroll_offset, 0.1f, 10000.0f );
    animations::lerp_to( HASH_CT( "tab__fade__" ) + HASH( this->name.c_str( ) ), this->is_selected, 0.1f );

    // Render fade & text.
    render::string( fonts::font_awesome, tab_icon_position, color{ 150, 150, 150, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), this->icon, true );
    render::string( fonts::visuals_segoe_ui, tab_text_position, color{ 150, 150, 150, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), this->name, true );

    if ( !this->is_selected )
        return;

    render::scissor_rect( parent_window->position.x, parent_window->position.y + parent_window->tab_height + parent_window->titlebar_size.y, parent_window->size.x, parent_window->size.y - parent_window->tab_height - ( parent_window->titlebar_size.y * 2 ), [ & ] {
        if ( this->num_columns ) {
            for ( int i = { }; i < this->num_columns; i++ ) {
                for ( int n = { }; n < this->child_windows[ i ].size( ); n++ ) {
                    auto &object = this->child_windows[ i ][ n ];

                    object->paint( );
                }
            }
        }
    } );
}

void penumbra::tab::input( ) {
    globals::should_scroll_window = true;

    auto parent = this->get_parent< penumbra::window >( );

    if ( !this->is_selected )
        return;

    const auto animation_scroll = animations::get( HASH_CT( "tab__scroll__" ) + HASH( this->name.c_str( ) ), 0.0f );

    if ( this->num_columns ) {
        auto column_width = ( parent->size.x / this->num_columns ) - 5.0f;

        this->highest_column_height = 0.0f;

        for ( int i = 0; i < this->num_columns; i++ ) {
            this->total_cursor_offset[ i ] = 0.0f;

            for ( int n = 0; n < this->child_windows[ i ].size( ); n++ ) {
                auto &object = this->child_windows[ i ][ n ];
               
                object->window_height = object->cursor_position.y + 17.0f;

                auto animation = animations::get( HASH_CT( "child__lerp__" ) + HASH( ( object->name + object->element_id ).c_str( ) ), object->window_height );
                       
                if ( object->is_collapsed )
                    object->window_height = 10.0f;

                object->padding = parent->size.y - 15.0f;

                this->total_cursor_offset[ i ] += object->window_height + 10.0f;

                if ( highest_column_height < total_cursor_offset[ i ] ) {
                    highest_column_height = total_cursor_offset[ i ];

                    highest_column = i;
                    last_control = object.get( );
                }

                this->padding = highest_column_height - object->padding;

                object->position = {
                        parent->position.x + 10.5f + ( column_width * i ),
                        parent->position.y + parent->tab_height + 30.0f + total_cursor_offset[ i ] - object->window_height - animation_scroll.value
                };

                object->size = {
                        ( parent->size.x / this->num_columns ) - 15.0f,
                        animation.value
                };

                object->input( );
            }
        }

        
        int height = ( parent->titlebar_size.y * 2.2f ) + parent->tab_height + this->padding;

        if ( parent->size.y > highest_column_height + height ) {
            this->scroll_offset = 0.0f;
            globals::should_scroll_window = false;
        }

            if ( !get_focused< object >( ) && globals::should_scroll_window ) {
                int max_height = parent->size.y > highest_column_height + height ? parent->size.y : highest_column_height + height;

                if ( globals::scroll_delta != 0.0 ) {
                    this->scroll_offset += globals::scroll_delta * 20.f;
                }

                this->scroll_offset = std::clamp< float >( this->scroll_offset, 0, max_height - highest_column_height );

                globals::scroll_delta = 0.0;
            }
        }
}
