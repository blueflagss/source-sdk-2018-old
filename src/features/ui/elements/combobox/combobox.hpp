#pragma once
#include <vector>

namespace penumbra
{
#define COMBOBOX_OFFSET \
    glm::vec2 { 0.f, 15.f }

    class combobox : public object {
    public:
        combobox( const std::string &name, int *value, const std::deque< std::string > &items ) {
            this->element_type = TYPE_COMBOBOX;
            this->name = name;
            this->value = value;
            this->items = items;
        }

        void animate( );
        void arrow( int x, int y, color color, float rotation );
        void paint( ) override;
        void input( ) override;

    private:
        int *value = nullptr;
        bool _is_focused = false;
        bool opened = false;
        bool _hovered_in_dropdown = false;
        std::deque< std::string > items = { };
    };
}// namespace xpui
