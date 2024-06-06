#pragma once

namespace penumbra
{
    class checkbox : public object {
    public:
        checkbox( const std::string &name, bool *enabled ) {
            std::vector< std::string > id_split = utils::split_str( name, '#' );

            this->name = id_split.at( 0 );

            if ( id_split.size( ) == 2 )
                this->element_id = id_split.at( 1 );

            this->element_type = TYPE_CHECKBOX;
            this->enabled = enabled;
        }

        void animate( );
        void render_checkmark( int x, int y, float scale, color color );
        void paint( ) override;
        void input( ) override;

    private:
        bool *enabled = nullptr;
    };
}// namespace xpui