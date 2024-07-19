#pragma once
#include <d3d9.h>
#include <glm/glm.hpp>
#include <blur/blur_binary.h>

namespace fonts
{
    inline ImFont *csgo_icons = nullptr;
    inline ImFont *montserrat = nullptr;
    inline ImFont *visuals_04b03 = nullptr;
    inline ImFont *visuals_segoe_ui = nullptr;
    inline ImFont *montserrat_main = nullptr;
    inline ImFont *visuals_indicators = nullptr;
    inline ImFont *montserrat_semibold = nullptr;
    inline ImFont *font_awesome = nullptr;
}// namespace fonts

namespace render
{
    inline IDirect3DDevice9 *device_ptr = nullptr;

    namespace blur_utils
    {
        static IDirect3DSurface9 *rtBackup = nullptr;
        static IDirect3DPixelShader9 *blurShaderX = nullptr;
        static IDirect3DPixelShader9 *blurShaderY = nullptr;
        static IDirect3DTexture9 *blurTexture = nullptr;
        static int backbufferWidth = 0;
        static int backbufferHeight = 0;

        static void BeginBlur( const ImDrawList *parent_list, const ImDrawCmd *cmd ) {
            if ( !blurShaderX ) {
                device_ptr->CreatePixelShader( reinterpret_cast< const DWORD * >( blur_x.data( ) ), &blurShaderX );
            }

            if ( !blurShaderY ) {
                device_ptr->CreatePixelShader( reinterpret_cast< const DWORD * >( blur_y.data( ) ), &blurShaderY );
            }

            IDirect3DSurface9 *backBuffer;
            device_ptr->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer );
            D3DSURFACE_DESC desc;
            backBuffer->GetDesc( &desc );

            if ( backbufferWidth != desc.Width || backbufferHeight != desc.Height ) {
                if ( blurTexture )
                    blurTexture->Release( );

                backbufferWidth = desc.Width;
                backbufferHeight = desc.Height;
                device_ptr->CreateTexture( desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &blurTexture, nullptr );
            }

            device_ptr->GetRenderTarget( 0, &rtBackup );

            {
                IDirect3DSurface9 *surface;
                blurTexture->GetSurfaceLevel( 0, &surface );
                device_ptr->StretchRect( backBuffer, NULL, surface, NULL, D3DTEXF_NONE );
                device_ptr->SetRenderTarget( 0, surface );
                surface->Release( );
            }

            backBuffer->Release( );

            device_ptr->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
            device_ptr->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
        }

        static void FirstBlurPass( const ImDrawList *parent_list, const ImDrawCmd *cmd ) {
            device_ptr->SetPixelShader( blurShaderX );
            const float params[ 4 ] = { 1.0f / backbufferWidth };
            device_ptr->SetPixelShaderConstantF( 0, params, 1 );
        }

        static void SecondBlurPass( const ImDrawList *parent_list, const ImDrawCmd *cmd ) {
            device_ptr->SetPixelShader( blurShaderY );
            const float params[ 4 ] = { 1.0f / backbufferHeight };
            device_ptr->SetPixelShaderConstantF( 0, params, 1 );
        }

        static void EndBlur( const ImDrawList *parent_list, const ImDrawCmd *cmd ) {
            device_ptr->SetRenderTarget( 0, rtBackup );
            rtBackup->Release( );

            device_ptr->SetPixelShader( nullptr );
            device_ptr->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
            device_ptr->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
        }
    }// namespace blur

    void init( HWND hwnd, IDirect3DDevice9 *device );
    void show_objects( );
    void blur( int x, int y, int w, int h, color col, float rounding = 0.0f );
    void blur( glm::vec2 pos, glm::vec2 size, color col, float rounding = 0.0f );
    void start( );
    void finish( );

    void string( ImFont *font, float x, float y, color col, const std::string &text, bool shadow = false, bool outline = false );
    void string( ImFont *font, glm::vec2 pos, color col, const std::string &text, bool shadow = false, bool outline = false );

    glm::vec2 get_text_size( ImFont *font, const std::string &text );

    void filled_rect( int x, int y, int w, int h, color col, float rounding = 0.f, int corner_flags = 0 );
    void filled_rect( glm::vec2 pos, glm::vec2 size, color col, float rounding = 0.f, int corner_flags = 0 );

    ImFont *create_from_system( const std::string &font_path, float size, const int flags, const ImWchar *glyph_ranges );
    ImFont *create_from_system( std::span< uint32_t > data, float size, const int flags, const ImWchar *glyph_ranges );
    ImFont *create_from_system( std::span< uint32_t > data, float size, const int flags, const ImWchar *glyph_ranges, ImFontConfig *cfg );

    void gradient_rect( int x, int y, int w, int h, color c1, color c2, bool vertical );
    void gradient_rect( glm::vec2 pos, glm::vec2 size, color c1, color c2, bool vertical );

    void line( int x, int y, int x0, int y0, color color, float thickness = 1.f );
    void line( glm::vec2 point_a, glm::vec2 point_b, color color, float thickness = 1.f );

    void multi_gradient_rect( int x, int y, int w, int h, color c1, color c2, color c3, color c4 );
    void multi_gradient_rect( glm::vec2 pos, glm::vec2 size, color c1, color c2, color c3, color c4 );

    void filled_triangle( int x1, int y1, int x2, int y2, int x3, int y3, color col );
    void filled_triangle( std::array< glm::vec2, 3 > points, color col );

    void triangle( int x1, int y1, int x2, int y2, int x3, int y3, color col, float thickness = 1.f );
    void triangle( std::array< glm::vec2, 3 > points, color col, float thickness = 1.f );

    void begin( const std::function< void( ) > &fn );

    void rect( int x, int y, int w, int h, color col, float rounding = 0.f, int corner_flags = 0 );
    void scissor_rect( float x, float y, float w, float h, const std::function< void( ) > &func );
    void scissor_rect( glm::vec2 pos, glm::vec2 size, const std::function< void( ) > &func );
    void rect( glm::vec2 pos, glm::vec2 size, color col, float rounding = 0.f, int corner_flags = 0 );

    inline IDirect3DStateBlock9 *pixel_state = nullptr;
    inline IDirect3DVertexDeclaration9 *vertex_declaration = nullptr;
    inline IDirect3DVertexShader9 *vertex_shader = nullptr;

    inline unsigned long old_D3DRS_COLORWRITEENABLE = 0;
    inline unsigned long srgbwrite = 0;
    inline ImDrawList *draw_list = nullptr;

    void begin_draw_states( IDirect3DDevice9 *device );
    void finish_draw_states( IDirect3DDevice9 *device );

    bool world_to_screen( const glm::vec3 &point, glm::vec2 &screen );

    void circle_filled( glm::vec2 pos, int radius, int segments, color col );

    void circle( glm::vec2 pos, int radius, int segments, color col, float thickness = 1.0f );
    void circle_filled( int x, int y, int radius, int segments, color col );
    void circle( int x, int y, int radius, int segments, color col, float thickness = 1.0f );
}// namespace render
