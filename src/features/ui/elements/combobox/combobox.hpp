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
            this->dropdown_height = 100.0f;
        }

        void animate( );
        void arrow( int x, int y, color color, float rotation );
        void paint( ) override;
        void input( ) override;

        inline void add_item( const std::string &text ) {
            this->items.push_back( text );
        }

        inline void delete_item( const std::string &text ) {
            this->items.pop_back( );
        }

        inline void clear( ) {
            this->items.clear( );
        }

        std::deque< std::string > items = { };
        int *value = nullptr;

    private:
        bool is_focused = false;
        bool opened = false;
        bool should_scroll = false;

        float list_item_height = 17.0f;
        float scroll_offset = 0.f;
        float scroll_fraction = 0.f;
        float dropdown_height = 0.0f;
        int offset = 0;

        bool hovered_in_dropdown = false;

    };
}// namespace xpui
