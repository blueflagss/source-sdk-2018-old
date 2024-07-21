#pragma once
#include <globals.hpp>

namespace penumbra
{
    enum focus_type : int {
        FOCUS_NONE = -1,
        FOCUS_BOX,
        FOCUS_OPACITY,
        FOCUS_HUE
    };

    class colorpicker : public object {
    public:
        colorpicker( const std::string &name, color *value, bool use_opacity = true ) {
            this->element_type = TYPE_COLORPICKER;
            this->name = name;
            this->size = glm::vec2{ 14.0f, 14.0f };

            this->use_opacity = use_opacity;
            this->value = value;
            this->color_hsv = hsv::from_rgb(*value);
            this->focused_type = FOCUS_NONE;
        }

        void animate( );
        void paint( ) override;
        void input( ) override;

    private:
        hsv color_hsv = { };
        focus_type focused_type = { };

        glm::vec2 opacity_mouse_position = { };
        glm::vec2 hue_mouse_position = { };
        glm::vec2 main_rectangle_mouse_position = { };

        glm::vec2 rectangle_opacity_dimensions = { };
        glm::vec2 rectangle_opacity_pos = { };
        glm::vec2 rectangle_hue_dimensions = { };
        glm::vec2 rectangle_hue_pos = { };
        glm::vec2 rectangle_picker_dimensions = { };
        glm::vec2 rectangle_picker_pos = { };
        glm::vec2 rectangle_dimensions = { };
        glm::vec2 rectangle_pos = { };

        color *value = nullptr;
        bool use_opacity = false;
        bool is_dragging = false;
        bool is_opened = false;
        bool _hovered_in_colorpicker_box = false;
    };
}// namespace xpui