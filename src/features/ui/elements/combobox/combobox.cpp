#include <array>
#include <elements.hpp>

void penumbra::combobox::animate( )
{
    animations::lerp_to( HASH_CT( "combobox__hovered__" ) + HASH( this->name.c_str( ) ), get_focused< object >( ) == this || this->is_hovered, 0.1f );
    animations::lerp_to( HASH_CT( "combobox__active__" ) + HASH( this->name.c_str( ) ), this->opened, 0.13f );
    animations::lerp_to( HASH_CT( "combobox__lerp__" ) + HASH( this->name.c_str( ) ), this->scroll_offset, 0.1f, 100000.0f );
}

void penumbra::combobox::arrow( int x, int y, color color, float rotation )
{
    constexpr int size = 3;

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

void penumbra::combobox::paint( )
{
    this->animate( );

    const auto selection_scroll_lerp = animations::get( HASH_CT( "combobox__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );
    const auto animation = animations::get( HASH_CT( "combobox__hovered__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), 0.0f );
    const auto animation_active = animations::get( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), 0.0f );

    this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

    render::filled_rect( this->position + COMBOBOX_OFFSET, this->size, color{ 32, 32, 32, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 24, 24, 24, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 2.0f );
    render::rect( this->position + COMBOBOX_OFFSET, this->size, color{ 60, 60, 60, 100 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 50, 50, 50, 100 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 2.0f );

    std::string item_name{ this->items.empty( ) ? "None" : this->items[ *this->value ] };

    auto item_dimensions = render::get_text_size( fonts::visuals_segoe_ui, item_name );

    render::scissor_rect( this->position + COMBOBOX_OFFSET, this->size, [ & ] {
        render::string( fonts::visuals_segoe_ui, this->position.x + 7.0f, this->position.y + COMBOBOX_OFFSET.y + 1.0f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 255, 255, 255, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_active.value ), item_name );
    } );

    render::string( fonts::visuals_segoe_ui, this->position.x - 0.5f, this->position.y - 5.5f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name );
    arrow( this->position.x + this->size.x - 11.0f, this->position.y + COMBOBOX_OFFSET.y + 8.5f, color{ 90, 90, 90, 255 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 180, 180, 180, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, animation_active.value ), -90.0f );

    if ( animation_active.value <= 0.03f )
        return;

    const auto dropdown_position = glm::vec2{ this->position.x, this->position.y + COMBOBOX_OFFSET.y + this->size.y - 1 };
    const auto dropdown_height_frac = this->dropdown_height * animation_active.value;

    render::scissor_rect( dropdown_position, glm::vec2{ this->size.x, dropdown_height_frac - 1.0f }, [ & ] {
        render::filled_rect( dropdown_position, glm::vec2{ this->size.x, dropdown_height_frac }, color{ 24, 24, 24, 255 * animation_active.value * globals::fade_opacity[ this->get_main_window( ) ] } );
        render::rect( dropdown_position, glm::vec2{ this->size.x, dropdown_height_frac }, color{ 60, 60, 60, 100 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 50, 50, 50, 100 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ) );
        
        if ( this->should_scroll )
            render::filled_rect( dropdown_position.x + this->size.x - 3, dropdown_position.y, 3, this->dropdown_height, color{ 38, 38, 38, 255 * animation_active.value * globals::fade_opacity[ this->get_main_window( ) ] } );

        const auto viewable_ratio = this->dropdown_height / selection_scroll_lerp.value;
        const auto scrollbar_area = 10.0f;
        const auto track_space = selection_scroll_lerp.value - this->dropdown_height;
        const auto thumb_space = this->dropdown_height - scrollbar_area;
        const auto max_height = dropdown_height > this->offset ? dropdown_height : this->offset;
        const auto percentage_scrolled = selection_scroll_lerp.value / ( max_height - this->dropdown_height );

        for ( int i = { }; i < this->items.size( ); i++ ) {
            std::vector< std::string > id_split = utils::split_str( this->items[ i ], '#' );
            std::string id = "";

            if ( id_split.size( ) == 2 )
                id = id_split.at( 1 );

            const auto item_name = id_split.at( 0 );
            const auto selection_animation = animations::get( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id + id ).c_str( ) ) + HASH( std::to_string( i ).c_str( ) ), 0.0f );
            auto hovered_over_item = input::in_region( dropdown_position + glm::vec2{ 0.f, ( this->size.y * i ) - selection_scroll_lerp.value }, this->size );

            auto per_item_dimensions = render::get_text_size( fonts::visuals_segoe_ui, this->items[ i ] );
            auto item_position = glm::vec2{ dropdown_position.x + 8.0f, dropdown_position.y + ( this->size.y * i ) - selection_scroll_lerp.value - 1.5f + ( this->size.y / 2 ) - per_item_dimensions.y / 2 };
            bool item_selected = hovered_over_item || *this->value == i;

            animations::lerp_to( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id + id ).c_str( ) ) + HASH( std::to_string( i ).c_str( ) ), item_selected, 0.14f );

            render::string( fonts::visuals_segoe_ui, item_position, color{ 200, 200, 200, 255 * animation_active.value * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ globals::theme_accent, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, selection_animation.value ), this->items[ i ] );
        }
  
        render::filled_rect( dropdown_position.x + this->size.x - 3, dropdown_position.y + percentage_scrolled * this->dropdown_height - scrollbar_area, 3, scrollbar_area, globals::theme_accent );    
    } );
}

void penumbra::combobox::input( )
{
    const auto animation_active = animations::get( HASH_CT( "combobox__active__" ) + HASH( ( this->name + this->element_id ).c_str( ) ), 0.0f );

    auto dropdown_position = glm::vec2{ this->position.x, this->position.y + COMBOBOX_OFFSET.y + this->size.y - 1 };

    this->hovered_in_dropdown = input::in_region( dropdown_position, glm::vec2{ this->size.x, this->dropdown_height * animation_active.value } );

    if ( !this->blocked( ) ) {
        this->is_hovered = input::in_region( this->position + COMBOBOX_OFFSET, this->size );

        if ( this->is_hovered && input::key_pressed( VK_LBUTTON ) ) {
            add_object_to_focus( this );
            this->opened = true;
        }
    }

    this->dropdown_height = 150.0f;

    const auto should_scroll = this->dropdown_height < float( this->size.y * this->items.size( ) );

    this->should_scroll = should_scroll;

    if ( !should_scroll )
        this->dropdown_height = ( this->size.y * this->items.size( ) );

    const auto selection_scroll_lerp = animations::get( HASH_CT( "combobox__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );

    if ( !this->opened && get_focused< object >( ) == this && animation_active.value <= 0.10f ) {
        remove_object_focus( );
        this->scroll_offset = 0.0f;
    }

    this->offset = 0;

    if ( get_focused< object >( ) == this && !this->items.empty( ) ) {
        for ( int i = { }; i < this->items.size( ); i++ ) {
            auto hovered_over_item = input::in_region( dropdown_position + glm::vec2{ 0.f, ( this->size.y * i ) - selection_scroll_lerp.value }, this->size );

            if ( this->hovered_in_dropdown ) {
                globals::should_scroll_window = false;

                if ( hovered_over_item && input::key_pressed( VK_LBUTTON ) )
                    *this->value = i;
            }

            this->offset += this->size.y;
        }

        if ( should_scroll ) {
            int max_height = dropdown_height > this->offset ? dropdown_height : this->offset;

            if ( this->hovered_in_dropdown && globals::scroll_delta != 0.0 ) {
                this->scroll_offset += globals::scroll_delta * 20.f;
            }

            this->scroll_offset = std::clamp< float >( this->scroll_offset, 0, max_height - dropdown_height );
        }
    }

    if ( ( !this->hovered_in_dropdown || this->is_hovered ) && animation_active.value >= 0.08f && input::key_pressed( VK_LBUTTON ) ) {
        this->opened = false;
    }
}