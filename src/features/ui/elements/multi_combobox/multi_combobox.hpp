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

        bool show_fade = false;
        bool _is_focused = false;
        bool _hovered_in_dropdown = false;
    };
}// namespace xpui