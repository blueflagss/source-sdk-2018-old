#pragma once

namespace penumbra
{
    class label : public object {
    public:
        static void operator delete( void *ptr ) {
            ::operator delete( ptr );
        }

        ~label( ) {
            this->name = "";
            this->element_id = "";
            this->element_type = TYPE_NONE;
        }

        label( const std::string &name ) {
            std::vector< std::string > id_split = utils::split_str( name, '#' );

            if ( id_split.size( ) == 2 )
                this->element_id = id_split.at( 1 );

            this->name = id_split.at( 0 );
            this->element_type = TYPE_LABEL;
        }

        void paint( ) override;
        void input( ) override;

    private:
    };
}// namespace xpui