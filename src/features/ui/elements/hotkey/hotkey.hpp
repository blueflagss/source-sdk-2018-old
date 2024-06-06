#pragma once

namespace penumbra
{
#define HOTKEY_OFFSET glm::vec2 { 0.f, 0.f }

    enum keybind_focus_type : int {
        NONE = -1,
        FOCUS_SELECTION,
        FOCUS_BIND
    };

    class hotkey : public object {
    public:
        hotkey( const std::string &name, int *bind, int *toggle ) {
            this->element_type = TYPE_HOTKEY;
            this->name = name;
            this->_bind = bind;
            this->_toggle = toggle;
            this->focused_type = keybind_focus_type::NONE;
        }

        void animate( );
        void paint( ) override;
        void input( ) override;

        keybind_focus_type focused_type = keybind_focus_type::NONE;

    protected:
        std::string _hotkey_text = { };
        bool _is_set = false, _old_set = false;
        bool toggle_opened = false;

        int *_bind = nullptr;
        int *_toggle = nullptr;
    };
}// namespace xpui