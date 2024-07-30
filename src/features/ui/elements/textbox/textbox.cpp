#include <array>
#include <elements.hpp>

std::deque< penumbra::key_code_info > special_characters = {
        { 48, '0', ')' },
        { 49, '1', '!' },
        { 50, '2', '@' },
        { 51, '3', '#' },
        { 52, '4', '$' },
        { 53, '5', '%' },
        { 54, '6', '^' },
        { 55, '7', '&' },
        { 56, '8', '*' },
        { 57, '9', '(' },
        { 32, ' ', ' ' },
        { 192, '`', '~' },
        { 189, '-', '_' },
        { 187, '=', '+' },
        { 219, '[', '{' },
        { 220, '\\', '|' },
        { 221, ']', '}' },
        { 186, ';', ':' },
        { 222, '\'', '"' },
        { 188, ',', '<' },
        { 190, '.', '>' },
        { 191, '/', '?' } 
};

void penumbra::textbox::animate( ) {
    animations::lerp_to( HASH_CT( "textbox__hovered__" ) + HASH( this->name.c_str( ) ), this->opened || this->is_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "textbox__active__" ) + HASH( this->name.c_str( ) ), this->opened, 0.15f );

    auto item_dimensions = render::get_text_size( fonts::montserrat, *this->value );

    animations::lerp_to( HASH_CT( "textbox__lerp__" ) + HASH( this->name.c_str( ) ), this->line_position.x, 0.10f );
    animations::lerp_to( HASH_CT( "textboxline__lerp__" ) + HASH( this->name.c_str( ) ), this->line_size.x, 0.10f );
}

void penumbra::textbox::paint( ) {
    this->animate( );

    const auto animation = animations::get( HASH_CT( "textbox__hovered__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_active = animations::get( HASH_CT( "textbox__active__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_lerp = animations::get( HASH_CT( "textbox__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_line_lerp = animations::get( HASH_CT( "textboxline__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );

    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position + TEXTBOX_OFFSET, this->size, color{ 24, 24, 24, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 32, 32, 32, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 2.0f );
    render::rect( this->position + TEXTBOX_OFFSET, this->size, color{ 46, 46, 46, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );

    auto item_dimensions = render::get_text_size( fonts::visuals_segoe_ui, *this->value );

    render::filled_rect( animation_lerp.value, this->position.y + 13.0f + TEXTBOX_OFFSET.y - 9.5f, animation_line_lerp.value, 13.0f, color{ 0, 0, 0, 0 }.lerp( color{ globals::theme_accent, 100 * animation_active.value * globals::fade_opacity[ this->get_main_window( ) ] }, animation_active.value ) );
    render::string( fonts::visuals_segoe_ui, this->position.x + 8.0f, this->position.y + 5.5f + TEXTBOX_OFFSET.y - 4.f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_active.value ), *this->value );

    if ( !this->is_in_object )
        render::string( fonts::visuals_segoe_ui, this->position.x - 0.5f, this->position.y - 3.5f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name );
}

void penumbra::textbox::input( ) {
    auto textbox_position = glm::vec2{ this->position.x, this->position.y + TEXTBOX_OFFSET.y + this->size.y - 1 };

    if ( !this->blocked( ) || this->is_in_object ) {
        this->is_hovered = input::in_region( this->position + TEXTBOX_OFFSET, this->size );

        if ( this->is_hovered && input::key_pressed( VK_LBUTTON ) )
            this->opened = true;
    }

    const auto animation_active = animations::get( HASH_CT( "textbox__active__" ) + HASH( this->name.c_str( ) ), 0.0f );

    if ( this->opened ) {
        if ( ( !this->is_hovered && input::key_pressed( VK_LBUTTON ) ) || input::key_pressed( VK_RETURN ) || input::key_pressed( VK_ESCAPE ) )
            this->opened = false;

        if ( this->value->length( ) > 0 ) {
            if ( input::key_pressed( VK_BACK ) ) {
                this->last_time = GetTickCount( );
                this->value->pop_back( );
            }

            if ( !this->hold_state && ( GetTickCount( ) - this->last_time ) > 500 ) {
                if ( input::key_down( VK_BACK ) )
                    this->hold_state = true;
            }

            if ( this->hold_state && ( GetTickCount( ) - this->last_time ) > 60 && input::key_down( VK_BACK ) ) {
                this->value->pop_back( );

                this->last_time = GetTickCount( );
            }
        }

        if ( this->value->length( ) > 0 ) {
            if ( !this->is_selected && input::key_down( VK_CONTROL ) && input::key_down( 0x41 ) )
                this->is_selected = true;

            if ( this->hold_state && !input::key_down( VK_BACK ) )
                this->hold_state = false;

            if ( this->is_selected && input::key_pressed( VK_BACK ) ) {
                this->value->clear( );

                this->is_selected = false;
            }
        }

        if ( this->value->length( ) < this->max_length || this->value->length( ) > 0 ) {
            if ( !input::key_down( VK_CONTROL ) || !input::key_down( 0x41 ) ) {
                for ( int i = 32; i <= 222; i++ ) {
                    if ( ( i > 32 && i < 48 ) || ( i > 57 && i < 65 ) || ( i > 90 && i < 186 ) )
                        continue;

                    if ( this->is_selected && input::key_pressed( i ) ) {
                        this->value->clear( );

                        this->is_selected = false;
                    }

                    if ( i > 57 && i <= 90 ) {
                        if ( input::key_pressed( i ) )
                            *this->value += input::key_down( VK_SHIFT ) ? static_cast< char >( i ) : static_cast< char >( i + 32 );
                    } else {
                        if ( input::key_pressed( i ) ) {
                            for ( int j = 0; j < special_characters.size( ); j++ ) {
                                if ( special_characters[ j ].key == i )
                                    *this->value += input::key_down( VK_SHIFT ) ? special_characters[ j ].shift : special_characters[ j ].regular;
                            }
                        }
                    }
                }
            }

            auto item_dimensions = render::get_text_size( fonts::visuals_segoe_ui, *this->value );

            if ( !this->is_selected ) {
                this->line_position = glm::vec2{
                        this->position.x + 6.0f + item_dimensions.x + 3.0f,
                        this->position.y + 8.0f + TEXTBOX_OFFSET.y - 3.0f
                };

                this->line_size = {
                        2.0f,
                        5.0f,
                };
            } else {
                this->line_position = glm::vec2{
                        this->position.x + 8.0f,
                        this->position.y + 8.0f + TEXTBOX_OFFSET.y - 3.0f };

                this->line_size = {
                        item_dimensions.x + 2.0f,
                        10.0f,
                };
            }
        }
    }

    if ( !this->is_hovered && input::key_pressed( VK_LBUTTON ) )
        this->opened = false;
}
