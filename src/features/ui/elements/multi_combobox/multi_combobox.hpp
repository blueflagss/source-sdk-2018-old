#pragma once

namespace penumbra
{
    class multi_combobox : public object {
    public:
        multi_combobox( const std::string &name, std::deque< std::pair< std::string, bool * > > items ) {
            std::vector< std::string > id_split = utils::split_str( name, '#' );

            this->name = id_split.at( 0 );
            this->element_type = TYPE_MULTI_COMBOBOX;
            this->items = items;

            if ( id_split.size( ) == 2 ) // has id
                this->element_id = id_split.at( 1 );
        }

        void animate( );
        void arrow( int x, int y, color color, float rotation );
        void paint( ) override;
        void input( ) override;


        bool opened = false;
    private:
        std::deque< std::pair< std::string, bool * > > items = { };
        float list_item_height = 17.0f;
        float scroll_offset = 0.f;
        float scroll_fraction = 0.f;
        float dropdown_height = 0.0f;
        bool should_scroll = false;
        float offset = 0.0f;
        bool show_fade = false;
        bool is_focused = false;
        bool hovered_in_dropdown = false;
    };
}// namespace xpui