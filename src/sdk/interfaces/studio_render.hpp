#pragma once

class c_studio_render {
public:
    PAD( 0x250 );
    i_material *forced_material;

    void set_alpha_modulation( float opacity ) {
        return utils::get_method< void( __thiscall * )( void *, float ) >( this, 28 )( this, opacity );
    }

    void forced_material_override( i_material *mat ) {
        return utils::get_method< void( __thiscall * )( void *, i_material *, int, int ) >( this, 33 )( this, mat, 0, 0 );
    }
};