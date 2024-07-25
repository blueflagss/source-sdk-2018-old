#pragma once
#include <features/ui/input/input.hpp>
#include <features/ui/animations/animations.hpp>
#include <fmt/format.h>

namespace penumbra
{
#define SLIDER_OFFSET \
    glm::vec2 { 0.f, 15.f }

    template< typename T >
    class slider : public object {
    public:
        slider( const std::string &name, T *value, T min, T max, const std::string &precision ) {
            std::vector< std::string > id_split = utils::split_str( name, '#' );

            this->name = id_split.at( 0 );

            if ( id_split.size( ) == 2 )// has id
                this->element_id = id_split.at( 1 );

            this->value = value;
            this->min = min;
            this->max = max;
            this->element_type = TYPE_SLIDER;
            this->precision = precision;
        }

        void animate( ) {
            animations::lerp_to( HASH_CT( "slider__hovered__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), this->is_hovered || this->is_dragging, 0.1f );

            auto slider_width = this->size.x * ( *this->value - this->min ) / ( this->max - this->min );

            animations::lerp_to( HASH_CT( "slider__lerp__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), slider_width, 0.1f );
        }

        void paint( ) override {
            this->animate( );

            const auto animation = animations::get( HASH_CT( "slider__hovered__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), 0.0f );
            const auto animation_lerp = animations::get( HASH_CT( "slider__lerp__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), 0.0f );

            this->fade_fraction = std::clamp< float >( animation.value, 0.0f, 1.0f );

            std::string percentage_format = fmt::format( "{}{}", static_cast< int >( *this->value ), this->precision );

            render::filled_rect( this->position + SLIDER_OFFSET, this->size, color{ 47, 47, 47, 200 * globals::fade_opacity[ this->get_main_window( ) ] }.lerp( color{ 65, 65, 65, 200 * globals::fade_opacity[ this->get_main_window( ) ] }, this->fade_fraction ), 3.0f );
            
            if ( animation_lerp.value > 1.f )
                render::filled_rect( this->position.x, this->position.y + SLIDER_OFFSET.y, animation_lerp.value, this->size.y, color{ globals::theme_accent, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, 3.0f );

            auto percentage_dimensions = render::get_text_size( fonts::montserrat, percentage_format );
            auto center_object = glm::vec2{ this->position.x + ( this->size.x / 2 ) - percentage_dimensions.x / 2, this->position.y + SLIDER_OFFSET.y + ( this->size.y / 2 ) - percentage_dimensions.y / 2 };

            render::string( fonts::visuals_segoe_ui, this->position.x + this->size.x - percentage_dimensions.x, this->position.y - 4.5f, color{ 200, 200, 200, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, percentage_format );
            render::string( fonts::visuals_segoe_ui, this->position.x - 1.0f, this->position.y - 4.5f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name );
        }

        void input( ) override {
            const auto animation_lerp = animations::get( HASH_CT( "slider__lerp__" ) + HASH( this->element_id.c_str( ) ) + HASH( this->name.c_str( ) ), 0.0f );

            auto text_dim       = render::get_text_size( fonts::montserrat, this->name );
            auto mouse_position = input::get_mouse_position( );
            auto parent_window  = this->get_parent( );

            if ( !this->blocked( ) )
                this->is_hovered = input::in_region( this->position + SLIDER_OFFSET, this->size ) || input::in_region( this->position.x + animation_lerp.value - 3.5f, this->position.y + SLIDER_OFFSET.y - 1.5f, 9, 9 );

            if ( this->is_hovered && input::key_pressed( VK_LBUTTON ) )
                this->is_dragging = true;

            if ( this->is_dragging )
                *this->value = std::clamp< float >( ( mouse_position.x - this->position.x ) / this->size.x, 0.f, 1.f ) * ( this->max - this->min ) + this->min;

            if ( this->is_dragging && !input::key_down( VK_LBUTTON ) )
                this->is_dragging = false;
        }

        std::string precision = { };

    private:
        T min = { }, max = { };
        T *value = nullptr;

        bool is_dragging = false;
        glm::vec2 render_dimensions;
    };
}// namespace xpui