#pragma once

#include <deque>
#include <features/ui/input/input.hpp>
#include <memory>
#include <shared_mutex>

namespace penumbra
{
    inline void *focused = nullptr;

    enum object_type : int {
        TYPE_NONE = -1,
        TYPE_WINDOW,
        TYPE_TAB,
        TYPE_COLUMN,
        TYPE_CHILD,
        TYPE_BUTTON,
        TYPE_COLORPICKER,
        TYPE_CHECKBOX,
        TYPE_TEXTBOX,
        TYPE_COMBOBOX,
        TYPE_MULTI_COMBOBOX,
        TYPE_HOTKEY,
        TYPE_LABEL,
        TYPE_SLIDER,
        TYPE_LISTBOX,
        TYPE_INDICATOR
    };

    inline std::unordered_map< uint32_t, bool > window_focused;

    namespace math
    {
        inline constexpr float deg_to_rad( float x ) {
            return x * ( glm::pi< float >( ) / 180.f );
        }

        inline constexpr float rad_to_deg( float x ) {
            return x * ( 180.f / glm::pi< float >( ) );
        }
    }// namespace math

    class window;

    inline std::vector< std::shared_ptr< penumbra::window > > windows = { };

    class object : std::enable_shared_from_this< object > {
    public:
        glm::vec2 position = { };
        glm::vec2 size = { };
        glm::vec2 cursor_position = { };
        std::string name = "";
        std::string element_id = "";

        template< typename T, typename... Args >
        inline std::shared_ptr< T > add_object( Args &&...arguments ) {
            auto object = std::make_shared< T >( std::forward< Args >( arguments )... );
            object->set_parent( this );
            children_objects.push_back( object );
            return object;
        }

    public:
        virtual void paint( ) = 0;
        virtual void input( ) = 0;
        void handle( );

        std::deque< std::shared_ptr< object > > children_objects;

    public:

        bool is_selected = false, is_hovered = false;
        float fade_fraction = 0.0f;

        template< typename T = object >
        inline T *get_parent( ) {
            return reinterpret_cast< T * >( this->parent );
        }

        /* really unsafe way to get our parent window. */
        template< typename T = window >
        inline T *get_main_window( ) { 
            const auto current_parent = get_parent< object >( );

            if ( this->element_type == TYPE_CHILD )
                return current_parent->get_parent< T >( );

            if ( !current_parent )
                return reinterpret_cast< T * >( this );

            if ( current_parent ) {
                switch ( current_parent->element_type ) {
                    case TYPE_CHILD: {
                        return current_parent->get_parent< object >( )->get_parent< T >( );
                    } break;
                    case TYPE_INDICATOR:
                    case TYPE_TAB: {
                        return reinterpret_cast< T * >( current_parent );
                    } break;
                    case TYPE_COLORPICKER:
                    case TYPE_CHECKBOX:
                    case TYPE_LISTBOX:
                    case TYPE_LABEL: {
                        auto second_parent = current_parent->get_parent< object >( )->get_parent< object >( );

                        if ( second_parent )
                            return second_parent->get_parent< T >( );

                        return reinterpret_cast< T * >( current_parent->get_parent< object >( ) );
                    } break;
                    default: {
                        if ( current_parent->get_parent< window >( ) )
                            return current_parent->get_parent< T >( );
                        else
                            return reinterpret_cast< T * >( current_parent );
                    } break;
                }
            }

            return nullptr;
        }

        inline void remove_object_focus( ) {
            focused = nullptr;
        }

        inline void set_parent( object *obj ) {
            this->parent = obj;
        }

        inline bool window_focused( ) {
            if ( penumbra::windows.empty( ) )
                return false;

            auto &current = penumbra::windows.back( );

            return this->get_main_window( ) == current.get( );
        }

        inline bool blocked( ) {
            auto parent_object = get_parent< object >( );

            if ( !input::in_region( parent_object->position, parent_object->size ) || !this->window_focused( ) ) {
                this->is_hovered = false;
                return true;
            }

            return false;
        }

        float first_pad = 0.f, last_pad = 0.f;
        glm::vec2 addition_offset = { }, total_bounds = { };
        float last_click_time = 0.0f;
        object_type element_type = TYPE_NONE;

    private:
        object *parent = nullptr;
    };
}// namespace penumbra

namespace penumbra
{
    inline std::shared_ptr< penumbra::window > settings_window = nullptr;

    namespace globals
    {
        inline bool window_opened = false;
        inline bool settings_opened = false;
        inline bool should_scroll_window = false;
        inline double scroll_delta = 0.0;
        inline color theme_accent = color{ 203, 135, 212 };
        inline color copied_color = color{ };
        inline std::map< object *, float > fade_opacity;
        inline object *tooltip;
    }// namespace globals

    template< typename T >
    inline void add_object_to_focus( T *item ) {
        focused = reinterpret_cast< object * >( item );
    }

    template< typename T = object >
    inline T *get_focused( ) {
        return reinterpret_cast< T * >( focused );
    }
}// namespace penumbra