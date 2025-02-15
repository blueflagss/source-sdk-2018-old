#include "cs_base_weapon.hpp"

vector_3d c_cs_weapon_base::calculate_spread( int seed, float inaccuracy, float spread, bool revolver2 ) {
    float r1, r2, r3, r4, s1, c1, s2, c2;

    if ( !globals::local_weapon_data || !globals::local_weapon_data->bullets )
        return { };

    auto item_def_index = item_definition_index( );
    auto wpn_recoil_index = recoil_index( );

    math::random_seed( ( seed & 0xff ) + 1 );

    r1 = math::random_float( 0.0f, 1.0f );
    r2 = math::random_float( 0.0f, glm::pi< float >( ) * 2.0f );

    if ( globals::cvars::weapon_accuracy_shotgun_spread_patterns->get_int( ) > 0 )
        g_addresses.get_shotgun_spread.get< void( __stdcall * )( int, int, int, float *, float * ) >( )( item_def_index, 0, globals::local_weapon_data->bullets * wpn_recoil_index, &r4, &r3 );

    else {
        r3 = math::random_float( 0.0f, 1.0f );
        r4 = math::random_float( 0.0f, glm::pi< float >( ) * 2.0f );
    }

    if ( item_def_index == weapons::revolver && revolver2 ) {
        r1 = 1.0f - ( r1 * r1 );
        r3 = 1.0f - ( r3 * r3 );
    }

    else if ( item_def_index == weapons::negev && wpn_recoil_index < 3.0f ) {
        for ( int i{ 3 }; i > wpn_recoil_index; --i ) {
            r1 *= r1;
            r3 *= r3;
        }

        r1 = 1.0f - r1;
        r3 = 1.0f - r3;
    }

    c1 = std::cos( r2 );
    c2 = std::cos( r4 );
    s1 = std::sin( r2 );
    s2 = std::sin( r4 );

    return {
            ( c1 * ( r1 * inaccuracy ) ) + ( c2 * ( r3 * spread ) ),
            ( s1 * ( r1 * inaccuracy ) ) + ( s2 * ( r3 * spread ) ),
            0.f };
}

vector_3d c_cs_weapon_base::calculate_spread( int seed, bool revolver2 ) {
    return calculate_spread( seed, get_inaccuracy( ), get_spread( ), revolver2 );
}

c_cs_weapon_info *c_cs_weapon_base::get_weapon_data( ) {
    if ( *reinterpret_cast< unsigned short * >( reinterpret_cast< uintptr_t >( this ) + 0x31FC ) == -1 ) {
        return nullptr;
    }

    return utils::get_method< c_cs_weapon_info *( __thiscall * ) ( void * ) >( this, 446 )( this );
}

bool c_cs_weapon_base::is_grenade( ) {
    if ( !globals::local_weapon_data )
        return false;

    return globals::local_weapon_data->weapon_type == weapon_type::WEAPONTYPE_GRENADE;
}

wchar_t *c_cs_weapon_base::get_name( ) {
    auto item_definition = static_data( econ_item_view( this ) );

    if ( !item_definition )
        return nullptr;

    return g_interfaces.localize->find( item_definition->item_base_name( ) );
}

void c_cs_weapon_base::update_accuracy_penalty( ) {
    return utils::get_method< void( __thiscall * )( void * ) >( this, 471 )( this );
}

float c_cs_weapon_base::get_inaccuracy( ) {
    return utils::get_method< float( __thiscall * )( void * ) >( this, 469 )( this );
}

float c_cs_weapon_base::get_spread( ) {
    return utils::get_method< float( __thiscall * )( void * ) >( this, 439 )( this );
}

bool c_cs_weapon_base::is_base_combat_weapon( ) {
    if ( !this )
        return false;

    return utils::get_method< bool( __thiscall * )( void * ) >( this, 160 )( this );
}

bool c_cs_weapon_base::scoped_weapon( ) {
    return item_definition_index( ) == weapons::g3sg1 ||
           item_definition_index( ) == weapons::scar20 ||
           item_definition_index( ) == weapons::ssg08 ||
           item_definition_index( ) == weapons::awp ||
           item_definition_index( ) == weapons::ssg556 ||
           item_definition_index( ) == weapons::aug;
}

float c_cs_weapon_base::get_lowest_accuracy( ) {
    const auto info = get_weapon_data( );

    // todo: add option for auto scope
    if ( globals::local_player->flags( ) & player_flags::ducking )
        return ( scoped_weapon( ) && zoom_level( ) != 0 ) ? info->inaccuracy_crouch_alt : info->inaccuracy_crouch;

    return ( scoped_weapon( ) && zoom_level( ) != 0 ) ? info->inaccuracy_stand_alt : info->inaccuracy_stand;
}

c_econ_item_definition *c_cs_weapon_base::econ_item_view( c_cs_weapon_base *weapon ) {
    static auto econ_item_view = signature::find( "client.dll", "8B 81 ? ? ? ? 81 C1 ? ? ? ? FF 50 04 83 C0 40 C3" ).get< c_econ_item_definition *( __thiscall * ) ( void * ) >( );

    return econ_item_view( weapon );
}

c_econ_item_definition *c_cs_weapon_base::static_data( c_econ_item_definition *item ) {
    static auto static_data = signature::find( "client.dll", "55 8B EC 51 56 57 8B F1 E8 ? ? ? ? 0F B7 8E ? ? ?" ).get< c_econ_item_definition *( __thiscall * ) ( void * ) >( );

    return static_data( item );
}