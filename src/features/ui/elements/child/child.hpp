#pragma once

namespace penumbra
{
    class child : public object {
    public:
        child( const std::string &name, bool is_using_font ) {
            std::vector< std::string > id_split = utils::split_str( name, '#' );

            this->name = id_split.at( 0 );

            if ( id_split.size( ) == 2 )// has id
                this->element_id = id_split.at( 1 );

            this->element_type = TYPE_CHILD;
            this->using_custom_font = is_using_font;
            this->icon = "";
        }

        void animate( );
        void paint( ) override;
        void input( ) override;
        void handle_object( const std::shared_ptr< penumbra::object > &object );

    public:
        object *last_object = { };
        glm::vec2 titlebar_size = { }, add_position = { };

        float total_child_height = 0.0f;
        float window_height = 0.0f;

        bool is_collapsed = false;
        bool in_collapse_region = false;

protected:
        bool using_custom_font = false;
        glm::vec2 icon_dimensions = { };
        const char *icon;
    };
}// namespace xpui