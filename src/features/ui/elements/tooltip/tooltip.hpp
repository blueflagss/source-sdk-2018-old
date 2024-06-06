#pragma once
#include "object/object.hpp"

namespace penumbra
{
    class tooltip : public object {
    public:
        static void operator delete( void *ptr ) {
            ::operator delete( ptr );
        }

        ~tooltip( ) {
            this->name = "";
            this->element_id = "";
            this->element_type = TYPE_NONE;
        }

        tooltip( const std::string &name ) {
            std::vector< std::string > id_split = utils::split_str( name, '#' );

            if ( id_split.size( ) == 2 )
                this->element_id = id_split.at( 1 );

            this->name = id_split.at( 0 );
            this->element_type = TYPE_HOTKEY;
        }

        void paint( ) override;
        void input( ) override;

    private:
    };
}// namespace penumbra