#include <elements.hpp>

std::array< color, 7 > hue_colors = {
        color{ 255, 0, 0 },
        color{ 255, 255, 0 },
        color{ 0, 255, 0 },
        color{ 0, 255, 255 },
        color{ 0, 0, 255 },
        color{ 255, 0, 255 },
        color{ 255, 0, 0 } 
};

void penumbra::colorpicker::animate( ) {
    animations::lerp_to( HASH_CT( "colorpicker__hovered__" ) + HASH( this->name.c_str( ) ), this->is_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "colorpicker__active__" ) + HASH( this->name.c_str( ) ), this->is_opened, 0.01f );

    const auto animation_active2 = animations::get( HASH_CT( "colorpicker__active__" ) + HASH( this->name.c_str( ) ), 0.0f );

    animations::lerp_to( HASH_CT( "colorpicker__active2__" ) + HASH( this->name.c_str( ) ), this->is_opened, this->is_opened ? 0.15f : 0.09f );
}

void penumbra::colorpicker::paint( ) {
    this->animate( );

    auto draw_color_rectangle_squares = [ & ]( glm::vec2 pos, glm::vec2 size, bool vertical_squares = false ) {
        if ( globals::fade_opacity[ this->get_main_window( ) ] < 0.95f )
            return;

        const auto square_side_len = vertical_squares ? size.x * 0.5f : size.y * 0.5f;
        auto alpha_color_flip = false;
        const auto remaining_len = vertical_squares ? std::fmodf( size.y, square_side_len ) : std::fmodf( size.x, square_side_len );

        for ( auto i = 0.0f; i < ( vertical_squares ? size.y : size.x ); i += square_side_len ) {
            const auto calculated_len = i > ( vertical_squares ? size.y - square_side_len : size.x - square_side_len ) ? remaining_len : square_side_len;

            if ( vertical_squares ) {
                render::filled_rect( pos.x, pos.y + i, calculated_len, calculated_len, alpha_color_flip ? color{ 100, 100, 100, 255 * globals::fade_opacity[ this->get_main_window( ) ] } : color{ 15, 15, 15, 255 * globals::fade_opacity[ this->get_main_window( ) ] } );
                render::filled_rect( pos.x + calculated_len, pos.y + i, calculated_len, calculated_len, alpha_color_flip ? color{ 15, 15, 15, 255 * globals::fade_opacity[ this->get_main_window( ) ] } : color{ 100, 100, 100, 255 * globals::fade_opacity[ this->get_main_window( ) ] } );
            } else {
                render::filled_rect( pos.x + i, pos.y, calculated_len, calculated_len, alpha_color_flip ? color{ 100, 100, 100, 255 * globals::fade_opacity[ this->get_main_window( ) ] } : color{ 15, 15, 15, 255 * globals::fade_opacity[ this->get_main_window( ) ] } );
                render::filled_rect( pos.x + i, pos.y + calculated_len, calculated_len, calculated_len, alpha_color_flip ? color{ 15, 15, 15, 255 * globals::fade_opacity[ this->get_main_window( ) ] } : color{ 100, 100, 100, 255 * globals::fade_opacity[ this->get_main_window( ) ] } );
            }

            alpha_color_flip = !alpha_color_flip;
        }
    };

    const auto animation = animations::get( HASH_CT( "colorpicker__hovered__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_active = animations::get( HASH_CT( "colorpicker__active__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation_active2 = animations::get( HASH_CT( "colorpicker__active2__" ) + HASH( this->name.c_str( ) ), 0.0f );

    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    auto active_fraction = std::clamp< float >( animation_active.value, 0.0f, 1.0f );

    render::circle_filled( this->position.x - 4.0f, this->position.y + 5, 8.f, 30, color{ *this->value, this->value->a * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ *this->value, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ) );
    render::circle( this->position.x - 4.0f, this->position.y + 5, 8.f, 60, color{ *this->value, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, 1.5f );

    this->rectangle_pos = glm::vec2{ this->position.x - 10, this->position.y - 2 };
    this->rectangle_dimensions = glm::vec2{ 165.f * animation_active2.value, this->use_opacity ? 153.f * animation_active2.value : 140.f * animation_active2.value };

    if ( !this->is_opened && get_focused< object >( ) == this && animation_active2.value < 0.10f ) {
        remove_object_focus( );
        return;
    }

    if ( animation_active2.value > 0.05f ) {
        render::filled_rect( this->rectangle_pos.x - 5, this->rectangle_pos.y - 5, this->rectangle_dimensions.x + 5, this->rectangle_dimensions.y + 2, color{ 50, 50, 50, 130 * globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f );
        render::filled_rect( this->rectangle_pos.x - 5, this->rectangle_pos.y - 5, this->rectangle_dimensions.x + 5, this->rectangle_dimensions.y + 2, color{ 22, 22, 22, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight );
        render::rect( this->rectangle_pos.x - 5 - 1, this->rectangle_pos.y - 5 - 1, this->rectangle_dimensions.x + 5 + 2, this->rectangle_dimensions.y + 2 + 2, color{ 9, 9, 9, 50 * globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f );
        render::rect( this->rectangle_pos.x - 5, this->rectangle_pos.y - 5, this->rectangle_dimensions.x + 5, this->rectangle_dimensions.y + 2, color{ 100, 100, 100, 100 * globals::fade_opacity[ this->get_main_window( ) ] }, 4.0f );

        render::scissor_rect( this->rectangle_pos, this->rectangle_dimensions, [ & ] {
            this->rectangle_picker_pos = glm::vec2{ this->rectangle_pos.x, this->rectangle_pos.y };
            this->rectangle_picker_dimensions = glm::vec2{ 142.f * animation_active2.value, 130.f * animation_active2.value };

            auto rectangle_color_hsv = hsv{ this->color_hsv.h, 100, 100 };
            auto rectangle_default_color = color{ 0, 0, 0 };

            auto from_hsv = rectangle_color_hsv.to_rgb( );

            from_hsv.a = 255 * globals::fade_opacity[ this->get_main_window( ) ];

            render::multi_gradient_rect( this->rectangle_picker_pos, this->rectangle_picker_dimensions, color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, from_hsv, from_hsv, color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] } );
            render::multi_gradient_rect( this->rectangle_picker_pos, this->rectangle_picker_dimensions, color{ 0, 0, 0, 0 }, color{ 0, 0, 0, 0 }, color{ 0, 0, 0, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, color{ 0, 0, 0, 255 * globals::fade_opacity[ this->get_main_window( ) ] } );

            this->rectangle_hue_pos = glm::vec2{ this->rectangle_picker_pos.x + this->rectangle_picker_dimensions.x + 4.f, this->rectangle_picker_pos.y };
            this->rectangle_hue_dimensions = glm::vec2{ 12.f, this->rectangle_picker_dimensions.y - 3 };

            for ( int i = 0; i < 6; i++ )
                render::multi_gradient_rect( this->rectangle_hue_pos.x, this->rectangle_hue_pos.y + ( i * ( this->rectangle_hue_dimensions.y / 6 ) ), this->rectangle_hue_dimensions.x, this->rectangle_hue_dimensions.y / 6, color( hue_colors[ i ], 255 * globals::fade_opacity[ this->get_main_window( ) ] ), color( hue_colors[ i ], 255 * globals::fade_opacity[ this->get_main_window( ) ] ), color( hue_colors[ i + 1 ], 255 * globals::fade_opacity[ this->get_main_window( ) ] ), color( hue_colors[ i + 1 ], 255 * globals::fade_opacity[ this->get_main_window( ) ] ) );

            if ( this->use_opacity ) {
                this->rectangle_opacity_pos = glm::vec2{ rectangle_picker_pos.x, rectangle_picker_pos.y + rectangle_picker_dimensions.y + 3.f };
                this->rectangle_opacity_dimensions = glm::vec2{ rectangle_picker_dimensions.x, 10.f };

                draw_color_rectangle_squares( this->rectangle_opacity_pos, this->rectangle_opacity_dimensions );
                render::gradient_rect( this->rectangle_opacity_pos, this->rectangle_opacity_dimensions, color{ 0, 0, 0, 0 }, *this->value, false );
            }
        } );

        auto rectangle_hue_indicator_position = glm::vec2{
                this->rectangle_hue_pos.x + 6,
                std::clamp< float >( this->rectangle_hue_pos.y + ( std::clamp< float >( ( this->color_hsv.h / 360.f ), 0.f, 1.f ) * static_cast< float >( this->rectangle_hue_dimensions.y ) ), this->rectangle_hue_pos.y, this->rectangle_hue_pos.y + this->rectangle_hue_dimensions.y - 3.f ) };

        render::circle( rectangle_hue_indicator_position, 6.f, 50, color{ 255, 255, 255, 255 * animation_active2.value * globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );

        if ( this->use_opacity ) {
            auto rectangle_opacity_indicator_position = glm::vec2{
                    std::clamp< float >( this->rectangle_opacity_pos.x, this->rectangle_opacity_pos.x + ( std::clamp< float >( this->value->a / 255.f, 0.f, 1.f ) * static_cast< float >( this->rectangle_opacity_dimensions.x ) - 3 ), this->rectangle_opacity_pos.x + this->rectangle_opacity_dimensions.x ),
                    this->rectangle_opacity_pos.y + 5
            };

            render::circle( rectangle_opacity_indicator_position, 6.f, 50, color{ 255, 255, 255, 255 * animation_active2.value * globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );
        }

        auto rectangle_indicator_position = glm::vec2{
                std::clamp< float >( this->rectangle_picker_pos.x + 3 + ( std::clamp< float >( ( this->color_hsv.s / 100.f ), 0.f, 1.f ) * static_cast< float >( this->rectangle_picker_dimensions.x ) ), this->rectangle_picker_pos.x, this->rectangle_picker_pos.x + this->rectangle_picker_dimensions.x - 3.f ),
                std::clamp< float >( this->rectangle_picker_pos.y - 4 + ( std::clamp< float >( 1.0f - ( this->color_hsv.v / 100.f ), 0.f, 1.f ) * static_cast< float >( this->rectangle_picker_dimensions.y ) ), this->rectangle_picker_pos.y, this->rectangle_picker_pos.y + this->rectangle_picker_dimensions.y - 4.f ) 
        };

        render::circle( rectangle_indicator_position, 6.f, 50, color{ 255, 255, 255, 255 * animation_active2.value * globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );
    }
}

void penumbra::colorpicker::input( ) {
    auto mouse_position = input::get_mouse_position( );
    auto dim = render::get_text_size( fonts::montserrat, this->name );
    auto parent_object = this->get_parent( );

    this->_hovered_in_colorpicker_box = input::in_region( this->rectangle_pos, this->rectangle_dimensions );
    this->is_hovered = input::in_region( this->position.x - this->size.x + 2, this->position.y - 6, 17, 19 );

    if ( ( !get_focused< object >( ) && !parent_object->blocked( ) ) && this->is_hovered && input::key_pressed( VK_LBUTTON ) ) {
        this->is_opened = true;
        add_object_to_focus( this );
    }

    const auto animation_active2 = animations::get( HASH_CT( "colorpicker__active2__" ) + HASH( this->name.c_str( ) ), 0.0f );

    this->opacity_mouse_position = glm::vec2{ this->rectangle_opacity_pos.x + int( std::roundf( this->value->a / 255.f * this->rectangle_opacity_dimensions.x ) ), this->rectangle_opacity_pos.y / 2 };

    if ( !this->is_opened && animation_active2.value < 0.80f )
        return;

    if ( input::key_pressed( VK_LBUTTON ) ) {
        if ( input::in_region( this->rectangle_picker_pos, this->rectangle_picker_dimensions ) )
            this->focused_type = FOCUS_BOX;

        else if ( input::in_region( this->rectangle_hue_pos, this->rectangle_hue_dimensions ) )
            this->focused_type = FOCUS_HUE;

        if ( this->use_opacity && input::in_region( this->rectangle_opacity_pos, this->rectangle_opacity_dimensions ) )
            this->focused_type = FOCUS_OPACITY;
    }

    if ( this->is_dragging && !input::in_region( this->rectangle_picker_pos, this->rectangle_picker_dimensions ) && !input::key_down( VK_LBUTTON ) ) {
        this->is_dragging = false;
        this->focused_type = FOCUS_NONE;
    }

    if ( this->focused_type != FOCUS_NONE && input::key_down( VK_LBUTTON ) ) {
        this->is_dragging = true;

        switch ( this->focused_type ) {
            case FOCUS_HUE: {
                this->hue_mouse_position = glm::vec2{ this->rectangle_hue_dimensions.y / 2, std::clamp< float >( mouse_position.y, this->rectangle_hue_pos.y, this->rectangle_hue_pos.y + this->rectangle_hue_dimensions.y ) };
                this->color_hsv.h = 360 * ( std::clamp( ( this->hue_mouse_position.y - this->rectangle_hue_pos.y ) / ( this->rectangle_hue_dimensions.y - 1 ), 0.0f, 1.0f ) );
            } break;
            case FOCUS_OPACITY: {
                if ( this->use_opacity )
                    this->opacity_mouse_position = glm::vec2{ std::clamp< float >( mouse_position.x, this->rectangle_opacity_pos.x, this->rectangle_opacity_pos.x + this->rectangle_opacity_dimensions.x ), this->rectangle_opacity_dimensions.y / 2 };
            } break;
            case FOCUS_BOX: {
                auto mouse_delta = glm::vec2{ input::mouse_position.x - this->rectangle_picker_pos.x, mouse_position.y - this->rectangle_picker_pos.y };

                this->color_hsv.s = 100 * std::clamp< float >( mouse_delta.x / rectangle_picker_dimensions.x, 0.f, 1.f );
                this->color_hsv.v = 100 * ( 1.0f - std::clamp< float >( mouse_delta.y / rectangle_picker_dimensions.y, 0.f, 1.f ) );
            } break;
        }
    }

    if ( this->focused_type != FOCUS_NONE ) {
        auto new_color_from_hsv = hsv{ this->color_hsv.h, this->color_hsv.s, this->color_hsv.v };
        auto new_color = new_color_from_hsv.to_rgb( );

        new_color.a = this->use_opacity ? static_cast< float >( static_cast< float >( this->opacity_mouse_position.x - this->rectangle_opacity_pos.x ) / this->rectangle_opacity_dimensions.x * 255.f ) : 255;

        *this->value = new_color;
    }

    if ( !this->_hovered_in_colorpicker_box && animation_active2.value > 0.60f && input::key_pressed( VK_LBUTTON ) ) {
        this->focused_type = FOCUS_NONE;
        this->is_dragging = false;
        this->is_opened = false;
    }
}