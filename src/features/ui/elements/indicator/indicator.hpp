#pragma once
#include <features/ui/input/input.hpp>
#include <fmt/format.h>

namespace penumbra
{
#define INDICATOR_OFFSET \
    glm::vec2 { 0.f, 15.f }

    template< typename T >
    class indicator : public object {
    public:
        indicator( const std::string &name, T *value, T min, T max ) {
            this->value = value;
            this->min = min;
            this->max = max;
            this->element_type = TYPE_INDICATOR;
            this->name = name;
        }

        void animate( ) {
            auto indicator_width = this->size.x * ( *this->value - this->min ) / ( this->max - this->min );

            animations::lerp_to( HASH_CT( "indicator__lerp__" ) + HASH( this->name.c_str( ) ), indicator_width, 0.1f );
        }

        void paint( ) override {
            this->animate( );

            const auto animation_lerp = animations::get( HASH_CT( "indicator__lerp__" ) + HASH( this->name.c_str( ) ), 0.0f );

            render::filled_rect( this->position + INDICATOR_OFFSET, this->size, color{ 25, 25, 25, 255 *  globals::fade_opacity[ this->get_main_window( ) ] }, 3.0f );

            if ( *this->value > 0 )
                render::filled_rect( this->position.x, this->position.y + INDICATOR_OFFSET.y, animation_lerp.value, this->size.y, color{ globals::theme_accent, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, 3.0f );

            std::string percentage_format = fmt::format( "{}", static_cast< int >( *this->value ) );

            auto percentage_dimensions = render::get_text_size( fonts::montserrat, percentage_format );
            auto center_object = glm::vec2{ this->position.x + ( this->size.x / 2 ) - percentage_dimensions.x / 2, this->position.y + SLIDER_OFFSET.y + ( this->size.y / 2 ) - percentage_dimensions.y / 2 };

            render::string( fonts::montserrat, this->position.x + this->size.x - percentage_dimensions.x, this->position.y - 4.5f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, percentage_format );
            render::string( fonts::montserrat, this->position.x - 0.5f, this->position.y - 4.5f, color{ 200, 200, 200, 255 * globals::fade_opacity[ this->get_main_window( ) ] }, this->name );
        }

        void input( ) override {
        }

        T *value = nullptr;
        T min = { }, max = { };

    private:

        glm::vec2 render_dimensions;
    };
}// namespace xpui