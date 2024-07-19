#pragma once
#include <vector>

namespace penumbra
{
    #define LISTBOX_OFFSET glm::vec2 { 0.f, 15.f }

    class listbox : public object {
    public:
        listbox( const std::string &name, int *value, const std::deque< std::string > &items, bool filter_items, float list_height = 100.0f ) {
            this->element_type = TYPE_LISTBOX;
            this->name = name;
            this->value = value;
            this->height = list_height;
            this->filter_items = filter_items;
            this->items = items;

            if ( filter_items ) {
                this->text_box = this->add_object< penumbra::textbox >( name + _xs( "__textbox" ), &this->filter_text, 50 );
                this->text_box->is_in_object = true;
            }                
        }

        inline void add_item( const std::string& text ) {
            this->items.push_back( text );
        }

        inline void delete_item( const std::string &text ) {
            this->items.pop_back( );
        }

        inline void clear( ) {
            this->items.clear( );
            this->filtered_items.clear( );
        }

        void animate( );
        void paint( ) override;
        void input( ) override;

        float height;
        int *value = nullptr;
        std::deque< std::string > items = { };
        std::map< int, std::string > filtered_items;
    private:
        std::shared_ptr< penumbra::textbox > text_box;
        std::string filter_text;
        bool opened = false;
        bool filter_items = false;

        int offset = 0;
        bool should_scroll = false;

        float scroll_offset = 0.f;
        float scroll_fraction = 0.f;

        int iterator = 0;
        glm::vec2 listbox_dimensions = { };
    };
}// namespace penumbra
