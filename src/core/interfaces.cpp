#include "interfaces.hpp"

void interfaces::init( ) {
    g_addresses.update_animation_state = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? 0F 57 C0 0F 11 86" ) ).add( 0x1 ).rel32( );
    g_addresses.get_shotgun_spread = signature::find( XOR( "client.dll" ), XOR( "E8 ? ? ? ? EB 38 83 EC 08" ) ).add( 0x1 ).rel32( );
    g_addresses.trace_to_studio_csgo_hitgroups_priority = signature::find( XOR( "client.dll" ), XOR( "55 8B EC 83 E4 ? 81 EC ? ? ? ? 8B C2" ) );
    g_addresses.tier0_allocated_thread_ids = address( reinterpret_cast<uint8_t*>(reinterpret_cast< uintptr_t >( GetModuleHandleA( "tier0.dll" ) ) + 0x51AA0) );

    engine_client = create_interface< c_engine_client * >( "engine.dll", HASH_CT( "VEngineClient014" ) );
    client = create_interface< c_base_client * >( "client.dll", HASH_CT( "VClient018" ) );
    surface = create_interface< c_surface * >( "vguimatsurface.dll", HASH_CT( "VGUI_Surface031" ) );
    entity_list = create_interface< c_entity_list * >( "client.dll", HASH_CT( "VClientEntityList003" ) );
    input_system = create_interface< c_input_system * >( "inputsystem.dll", HASH_CT( "InputSystemVersion001" ) );
    render_view = create_interface< c_render_view * >( "engine.dll", HASH_CT( "VEngineRenderView014" ) );
    model_cache = create_interface< c_model_cache * >( "datacache.dll", HASH_CT( "MDLCache004" ) );
    game_movement = create_interface< c_game_movement * >( "client.dll", HASH_CT( "GameMovement001" ) );
    model_render = create_interface< i_model_render * >( "engine.dll", HASH_CT( "VEngineModel016" ) );
    panel = create_interface< c_panel * >( "vgui2.dll", HASH_CT( "VGUI_Panel009" ) );
    engine_vgui = create_interface< c_engine_vgui * >( "engine.dll", HASH_CT( "VEngineVGui001" ) );
    engine_trace = create_interface< c_engine_trace * >( "engine.dll", HASH_CT( "EngineTraceClient004" ) );
    event_manager = create_interface< c_game_event_mgr * >( "engine.dll", HASH_CT( "GAMEEVENTSMANAGER002" ) );
    material_system = create_interface< i_material_system * >( "materialsystem.dll", HASH_CT( "VMaterialSystem080" ) );
    physics_props = create_interface< c_physics_props * >( "vphysics.dll", HASH_CT( "VPhysicsSurfaceProps001" ) );
    cvar = create_interface< i_cvar * >( "vstdlib.dll", HASH_CT( "VEngineCvar007" ) );
    prediction = create_interface< c_prediction * >( "client.dll", HASH_CT( "VClientPrediction001" ) );
    model_info = create_interface< c_model_info * >( "engine.dll", HASH_CT( "VModelInfoClient004" ) );
    debug_overlay = create_interface< i_debug_overlay * >( "engine.dll", HASH_CT( "VDebugOverlay004" ) );
    mem_alloc = *reinterpret_cast< c_mem_alloc ** >( GetProcAddress( GetModuleHandleA( "tier0.dll" ), "g_pMemAlloc" ) );
    localize = create_interface< c_localize * >( "localize.dll", HASH_CT( "Localize_001" ) );
    studio_render = create_interface< c_studio_render * >( "engine.dll", HASH_CT( "VStudioRender026" ) );

    client_mode = **reinterpret_cast< c_client_mode *** >( utils::get_method< std::uintptr_t >( client, 10 ) + 0x5 );
    global_vars = signature::find( "client.dll", "A1 ? ? ? ? FF 70 04 68 ? ? ? ? 56 E8 ? ? ? ? 8B 06" ).add( 0x1 ).deref( ).deref( ).get< c_global_vars_base * >( );
    glow_object_manager = signature::find( "client.dll", "0F 11 05 ?? ?? ?? ?? 83 C8 01 C7 05" ).add( 0x3 ).deref( ).get< c_glow_object_mgr * >( );
    device = signature::find( "shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C" ).add( 0x1 ).deref( ).deref( ).get< IDirect3DDevice9 * >( );
    input = signature::find( "client.dll", "B9 ? ? ? ? FF 60 60" ).add( 0x1 ).deref( ).get< c_input * >( );
    client_state = signature::find( "engine.dll", "A1 ? ? ? ? 8B 88 ? ? ? ? 85 C9 75 07" ).add( 0x1 ).deref( ).deref( ).get< c_client_state * >( );

    globals::cvars::init( );

    //const auto new_pipe = steam_client->CreateSteamPipe( );

    //if ( new_pipe ) {
    //    const auto new_user = steam_client->ConnectToGlobalUser( new_pipe );

    //    if ( new_user ) {
    //        steam_friends = steam_client->GetISteamFriends( new_user, new_pipe, STEAMFRIENDS_INTERFACE_VERSION );
    //        steam_utils = steam_client->GetISteamUtils( new_user, STEAMUTILS_INTERFACE_VERSION );
    //    }
    //}

    if ( !window_handle )
        window_handle = FindWindowA( nullptr, "Counter-Strike: Global Offensive" );

    g_interfaces.engine_client->client_cmd_unrestricted( "rate 786432" );
}

template< typename T >
T interfaces::create_interface( const char *module, const hash32_t &interface_name ) {
    using interface_callback_fn = void *( __cdecl * ) ( );

    struct interface_register {
        interface_callback_fn callback;
        const char *name;
        interface_register *next;
    };

    auto create_interface_export = reinterpret_cast< std::uint8_t * >( GetProcAddress( GetModuleHandleA( module ), "CreateInterface" ) );

    if ( !create_interface_export )
        return nullptr;

    bool is_using_relative_call = false;

    while ( !( *reinterpret_cast< uint16_t * >( create_interface_export ) == 0x358B ) ) {// mov esi, XXXXXXXX
        // relative jmp
        if ( *reinterpret_cast< uint8_t * >( create_interface_export ) == 0xE9 ) {
            is_using_relative_call = true;
            break;
        }

        create_interface_export++;
    }

    if ( is_using_relative_call ) {
        // get absolute address from jmp
        create_interface_export = address( create_interface_export ).add( 1 ).rel32( ).get< std::uint8_t * >( );

        // reach instruction again.
        while ( !( *reinterpret_cast< uint16_t * >( create_interface_export ) == 0x358B ) )
            create_interface_export++;
    }

    auto interface_list = address( create_interface_export ).add( 2 ).deref( ).deref( ).get< interface_register * >( );

    for ( auto it = interface_list; it; it = it->next ) {
        if ( HASH( it->name ) != interface_name ) {
#if 0
#ifdef _DEBUG
            spdlog::set_level( spdlog::level::debug );
            spdlog::debug( "interface: {}", it->name );
#endif
#endif
            continue;
        }

        auto callback = reinterpret_cast< T >( it->callback( ) );

        spdlog::info( "interface: captured \"{}\" at [{}]", it->name, reinterpret_cast< void * >( callback ) );

        return callback;
    }
}