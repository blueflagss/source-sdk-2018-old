#pragma once
#include <vector>

namespace penumbra
{
    struct key_code_info {
        int key;
        char regular;
        char shift;
    };

    #define TEXTBOX_OFFSET glm::vec2 { 0.f, 15.f }

    class textbox : public object {
    public:
        textbox( const std::string &name, std::string *value, const int max_length ) {
            this->element_type = TYPE_TEXTBOX;
            this->name = name;
            this->value = value;
            this->max_length = max_length;
            this->is_in_object = false;
        }

        void animate( );
        void paint( ) override;
        void input( ) override;

        bool opened = false;
        bool is_in_object = false;
    private:
        float last_time = 0.0f;
        std::string *value = nullptr;
        int max_length;
        glm::vec2 line_position, line_size;
        bool _is_focused = false;
        bool hold_state = false;
        bool select_state = false;
        bool _hovered_in_dropdown = false;
    };
}// namespace penumbra
