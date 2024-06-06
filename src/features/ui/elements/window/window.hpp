#pragma once

enum form_flags : int {
    WINDOW_NONE = -1,
    WINDOW_MAIN,
    WINDOW_INDICATOR
};

namespace penumbra
{
    class window : public object {
    public:
        window( const std::string &name, const std::string &game_name, bool *opened, glm::vec2 pos, glm::vec2 dim, form_flags flags ) {
            this->element_type = TYPE_WINDOW;
            this->name = name;
            this->position = pos;
            this->size = dim;
            this->_original_size = dim;
            this->game_name = game_name;
            this->opened = opened;
            this->flags = flags;
            this->should_disable_input = false;

            switch ( this->flags ) {
                case WINDOW_MAIN: {
                    this->is_using_font = false;
                    this->is_transparent = false;
                    this->is_window_resizeable = true;
                    this->style_button = true;
                } break;
                case WINDOW_INDICATOR: {
                    this->is_using_font = true;
                    this->is_transparent = true;
                    this->is_window_resizeable = false;
                } break;
            }

            this->tab_height = 42.0f;
        }

        inline std::shared_ptr< tab > add_tab( const std::string &name, const char *icon, int columns = 1 ) {
            auto object = std::make_shared< penumbra::tab >( name, icon, columns );
            object->set_parent( this );
            tab_objects.push_back( object );
            return object;
        }

        void animate( );
        void paint( ) override;
        void input( ) override;
        void handle_object( const std::shared_ptr< penumbra::object > &object );

        bool *opened = nullptr;

    private:
        bool is_window_resizeable = false;
        bool is_using_font = false;
        bool style_button = false;
        bool is_transparent = false;
        bool is_exit_hovered = false;
        bool is_style_hovered = false;
        bool is_dragging = false, is_resizing = false;
        bool is_resize_area_hovered = false;
    public:
        bool should_disable_input = false;

        form_flags flags = WINDOW_NONE; 
        bool is_visible = false;
        bool is_hovered_in_rectangle = false;
        int current_page = 0;

        glm::vec2 _original_size;
        glm::vec2 titlebar_size, drag_delta = { };
        glm::vec2 resize_delta = { };

        float tab_height = 0;
        float animated_tab_position = 0.0f;
        float window_height = 0.0f;
        float window_resize_height = 0.f;

        std::string game_name = { };
        std::deque< std::shared_ptr< tab > > tab_objects = { };
    };
}// namespace xpui