#pragma once

namespace penumbra
{
    class child;

    class tab : public object {
    public:
        tab( const std::string &name, const char *icon, int columns = 1 ) {
            this->element_type = TYPE_TAB;
            this->name = name;

            this->num_columns = columns;
            this->icon = icon;
        }

        inline std::shared_ptr< child > add_child( const std::string &name, int column, bool using_font = false ) {
            auto object = std::make_shared< child >( name, using_font );
            object->set_parent( this );
            child_windows[ column ].push_back( object );
            return object;
        }

        void animate_childs( );
        void paint( ) override;
        void input( ) override;
  
        int first_pad = 0, last_pad = 0;

        int highest_column = 0;
        float scroll_offset = 0.0f;
        float total_cursor_offset[ 3 ];
        float highest_column_height;
        const char *icon;
        int num_columns = 0;
        float fade_fraction = 0.0f;
        float padding = 0.0f;
        int current_page = 0;
        penumbra::child *last_control = nullptr;
    private:
        std::deque< std::shared_ptr< child > > child_windows[ 3 ] = { };
        float offset_per_column[ 3 ] = { };
    };
}// namespace xpui