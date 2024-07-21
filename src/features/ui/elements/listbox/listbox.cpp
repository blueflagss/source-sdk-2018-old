#include <array>
#include <elements.hpp>

void penumbra::listbox::animate( ) {
    animations::lerp_to( HASH_CT( "listbox__lerp__" ) + HASH( this->name.c_str( ) ), this->scroll_offset, 0.1f, 100000.0f );
}

void penumbra::listbox::paint( ) {
    auto listbox_position = glm::vec2{ this->position.x, this->position.y + LISTBOX_OFFSET.y };

    this->listbox_dimensions = glm::vec2{ this->size.x, this->height };

    render::string( fonts::visuals_segoe_ui, this->position.x - 0.5f, this->position.y - 4.5f, color{ 200, 200, 200, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, this->name );

    const auto selection_scroll_lerp = animations::get( HASH_CT( "listbox__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );

    bool flipped_color = false;

    render::scissor_rect( listbox_position.x, listbox_position.y + ( this->filter_items ? text_box->size.y : 0.0f ), this->listbox_dimensions.x, this->listbox_dimensions.y - ( this->filter_items ? text_box->size.y : 0.0f ), [ & ] {
        render::filled_rect( listbox_position, this->listbox_dimensions, color{ 25, 25, 25, 225 *  globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );

        this->iterator = 0;

        for ( const auto &item : this->filtered_items ) {
            const auto selection_animation = animations::get( HASH_CT( "listbox__active__" ) + HASH( this->name.c_str( ) ) + HASH( std::to_string( item.first ).c_str( ) ), 0.0f );

            auto per_item_dimensions = render::get_text_size( fonts::montserrat, item.second );
            auto item_position = glm::vec2{ listbox_position.x, ( listbox_position.y + ( ( this->filter_items ? this->text_box->size.y : 0.0f ) + ( this->list_item_height * iterator ) ) - selection_scroll_lerp.value ) };
            bool item_selected = !get_focused< object >( ) && input::in_region( item_position.x, item_position.y, this->listbox_dimensions.x, this->list_item_height );

            animations::lerp_to( HASH_CT( "listbox__active__" ) + HASH( this->name.c_str( ) ) + HASH( std::to_string( item.first ).c_str( ) ), ( item_selected || *this->value == item.first ), 0.1f );

            if ( !flipped_color )
                render::filled_rect( item_position.x, item_position.y, this->listbox_dimensions.x, this->list_item_height, color{ 9, 9, 9, 50 * globals::fade_opacity[ this->get_main_window( ) ] } );

            render::string( fonts::visuals_segoe_ui, item_position.x + 8.0f, item_position.y - 1.0f, color{ 100, 100, 100, 200 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ globals::theme_accent, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, selection_animation.value ), this->items[ item.first ] );

            flipped_color = !flipped_color;
            this->iterator++;
        }
    } );

    render::rect( listbox_position, this->listbox_dimensions, color{ 46, 46, 46, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 2.0f );

    if ( this->filter_items ) {
        this->text_box->position = listbox_position - TEXTBOX_OFFSET;

        this->text_box->size = glm::vec2{
                this->size.x,
                this->list_item_height
        };

        this->text_box->paint( );
    }
}

void penumbra::listbox::input( ) {
    auto listbox_position = glm::vec2{ this->position.x, this->position.y + LISTBOX_OFFSET.y };

    if ( !this->blocked( ) && !get_focused< object >( ) )
        this->is_hovered = input::in_region( listbox_position.x, listbox_position.y + ( this->filter_items ? text_box->size.y : 0.0f ), this->listbox_dimensions.x, this->listbox_dimensions.y - ( this->filter_items ? text_box->size.y : 0.0f ) );

    auto transform_lowercase = [ ]( const std::string &text ) -> std::string {
        std::string temp = text;

        std::transform( temp.begin( ), temp.end( ), temp.begin( ), ::tolower );

        return temp;
    };

    if ( this->text_box && this->filter_items ) {
        this->text_box->input( );
        this->filtered_items.clear( );

        if ( this->filter_text.length( ) > 0 ) {
            for ( int i = 0; i < this->items.size( ); i++ ) {
                if ( ( this->items[ i ].find( this->filter_text ) != std::string::npos ) || ( transform_lowercase( this->items[ i ] ).find( transform_lowercase( this->filter_text ) ) != std::string::npos ) )
                    this->filtered_items[ i ] = this->items[ i ];
            }
        }

        else {
            for ( int i = 0; i < this->items.size( ); i++ )
                this->filtered_items[ i ] = this->items[ i ];
        }
    } else {
        for ( int i = 0; i < this->items.size( ); i++ )
            this->filtered_items[ i ] = this->items[ i ];
    }

    this->iterator = 0; this->offset = 0;

    const auto selection_scroll_lerp = animations::get( HASH_CT( "listbox__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );

    for ( const auto &item : this->filtered_items ) {
        auto per_item_dimensions = render::get_text_size( fonts::visuals_segoe_ui, this->items[ item.first ] );
        auto item_position = glm::vec2{ listbox_position.x, listbox_position.y + ( this->filter_items ? text_box->size.y : 0.0f ) + ( this->list_item_height * this->iterator ) - selection_scroll_lerp.value };
        bool item_selected = !get_focused< object >( ) && input::in_region( item_position.x, item_position.y, this->listbox_dimensions.x, this->list_item_height );
        
        if ( this->is_hovered ) {
            globals::should_scroll_window = false;

            if ( item_selected && input::key_pressed( VK_LBUTTON ) && ( !this->blocked( ) && !get_focused< object >( ) ) ) {
                *this->value = item.first;
            }
        }

        this->offset += this->list_item_height;
        this->iterator++;
    }

    this->should_scroll = this->is_hovered && focused == nullptr;

    int max_height = this->height > this->offset + ( this->text_box->size.y ) ? this->height : this->offset + this->text_box->size.y;

    if ( this->should_scroll && this->is_hovered && globals::scroll_delta != 0.0 ) {
        this->scroll_offset += globals::scroll_delta * 20.f;
    }

    this->scroll_offset = std::clamp< float >( this->scroll_offset, 0, max_height - this->listbox_dimensions.y );
    this->animate( );
}