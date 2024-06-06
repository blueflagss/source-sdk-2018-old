#include <elements.hpp>
#include <array>

void penumbra::combobox::animate( ) {
    animations::lerp_to( HASH_CT( "combobox__hovered__" ) + HASH( this->name.c_str( ) ), get_focused< object >( ) == this || this->is_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "combobox__active__" ) + HASH( this->name.c_str( ) ), this->opened, 0.13f );
}

void penumbra::combobox::arrow( int x, int y, color color, float rotation ) {// was gonna make the arrow rotate but i said fukc that xD.
    constexpr int size = 5;

    const auto angle_1 = penumbra::math::deg_to_rad( -90.0f + rotation );
    const auto angle_2 = penumbra::math::deg_to_rad( 90.0f + rotation );
    const auto angle_3 = penumbra::math::deg_to_rad( 0.0f + rotation );

    std::array< glm::vec2, 3 > points = {
            glm::vec2( x - std::cos( angle_2 ) * size, y - std::sin( angle_2 ) * size ),
            glm::vec2( x - std::cos( angle_3 ) * size, y - std::sin( angle_3 ) * size ),
            glm::vec2( x - std::cos( angle_1 ) * size, y - std::sin( angle_1 ) * size )
    };

    render::filled_triangle( points, color );
}

void penumbra::combobox::paint( ) {
    this->animate( );

    const auto animation = animations::get( HASH_CT( "combobox__hovered__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), 0.0f );
    const auto animation_active = animations::get( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), 0.0f );

    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position + COMBOBOX_OFFSET, this->size, color{ 24, 24, 24, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 32, 32, 32, 200 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 2.0f );
    render::rect( this->position + COMBOBOX_OFFSET, this->size, color{ 9, 9, 9, 120 *  globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );

    auto item_dimensions = render::get_text_size( fonts::montserrat, this->items[ *this->value ] );

    render::scissor_rect( this->position + COMBOBOX_OFFSET, this->size, [ & ] {
        render::string( fonts::montserrat, this->position.x + 8.0f, this->position.y + 8.0f + COMBOBOX_OFFSET.y - 4.f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_active.value ), this->items[ *this->value ] );
    } );

    render::string( fonts::montserrat, this->position.x - 0.5f, this->position.y - 4.5f, color{ 200, 200, 200, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->name );

    arrow( this->position.x + this->size.x - 14.0f, this->position.y + COMBOBOX_OFFSET.y + 10.f, color{ 90, 90, 90, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 180, 180, 180, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, animation_active.value ), -90.0f );

    if ( animation_active.value <= 0.01f )
        return;

    auto dropdown_position = glm::vec2{ this->position.x, this->position.y + COMBOBOX_OFFSET.y + this->size.y };

    render::scissor_rect( dropdown_position, glm::vec2{ this->size.x, ( this->size.y * this->items.size( ) ) * animation_active.value }, [ & ] {
        render::filled_rect( dropdown_position, glm::vec2{ this->size.x, ( this->size.y * this->items.size( ) ) * animation_active.value }, color{ 9, 9, 9, 225 * animation_active.value *  globals::fade_opacity[ this->get_main_window( ) ] } );
        render::rect( dropdown_position, glm::vec2{ this->size.x, ( this->size.y * this->items.size( ) ) * animation_active.value }, color{ 9, 9, 9, 255 * animation_active.value *  globals::fade_opacity[ this->get_main_window( ) ] } );

        for ( int i = { }; i < this->items.size( ); i++ ) {
            std::vector< std::string > id_split = utils::split_str( this->items[ i ], '#' );
            std::string id = "";

            if ( id_split.size( ) == 2 )
                id = id_split.at( 1 );

            const auto item_name = id_split.at( 0 );
            const auto selection_animation = animations::get( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id + id ).c_str( ) ) + HASH( std::to_string( i ).c_str( ) ), 0.0f );

            auto per_item_dimensions = render::get_text_size( fonts::montserrat, this->items[ i ] );
            auto item_position = glm::vec2{ dropdown_position.x + 8.0f, dropdown_position.y + ( this->size.y * i ) - 1.5f + ( this->size.y / 2 ) - per_item_dimensions.y / 2 };
            bool item_selected = *this->value == i;

            animations::lerp_to( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id + id ).c_str( ) ) + HASH( std::to_string( i ).c_str( ) ), item_selected, 0.14f );
            render::string( fonts::montserrat, item_position, color{ 200, 200, 200, 255 * animation_active.value *  globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ globals::theme_accent, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, selection_animation.value ), this->items[ i ] );
        }
    } );
}

void penumbra::combobox::input( ) {
    auto dropdown_position = glm::vec2{ this->position.x, this->position.y + COMBOBOX_OFFSET.y + this->size.y - 1 };

    this->_hovered_in_dropdown = input::in_region( dropdown_position, glm::vec2{ this->size.x, this->size.y * this->items.size( ) } );

    if ( !this->blocked( ) ) {
        this->is_hovered = input::in_region( this->position + COMBOBOX_OFFSET, this->size );

        if ( this->is_hovered && input::key_pressed( VK_LBUTTON ) ) {
            add_object_to_focus( this );
            this->opened = true;
        }
    }

    const auto animation_active = animations::get( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), 0.0f );

    if ( !this->opened && get_focused< object >( ) == this && animation_active.value <= 0.08f )
        remove_object_focus( );
    
    if ( get_focused< object >( ) == this ) {
        for ( int i = { }; i < this->items.size( ); i++ ) {
            auto hovered_over_item = input::in_region( dropdown_position + glm::vec2{ 0.f, this->size.y * i }, this->size );

            if ( hovered_over_item && input::key_pressed( VK_LBUTTON ) )
                *this->value = i;
        }
    }

    if ( ( !this->_hovered_in_dropdown || this->is_hovered ) && animation_active.value >= 0.08f && input::key_pressed( VK_LBUTTON ) )
        this->opened = false;
}