#pragma once

#include <functional>

namespace penumbra
{
    class button : public object {
    public:
        button( const std::string &name, const std::function< void( ) > &callback ) {
            std::vector< std::string > id_split = utils::split_str( name, '#' );

            this->element_type = TYPE_BUTTON;

            this->name = id_split.at( 0 );

            if ( id_split.size( ) == 2 )// has id
                this->element_id = id_split.at( 1 );

            this->enabled = enabled;
            this->callback = callback;
        }

        void animate( );

        void paint( ) override;
        void input( ) override;

    private:
        bool *enabled = nullptr;
        std::function< void( ) > callback = { };
    };
}// namespace xpui