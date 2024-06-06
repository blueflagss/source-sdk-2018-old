#pragma once
#include <d3d9.h>
#include <glm/glm.hpp>

namespace fonts
{
    inline ImFont *montserrat = nullptr;
    inline ImFont *visuals_04b03 = nullptr;
    inline ImFont *visuals_segoe_ui = nullptr;
    inline ImFont *montserrat_main = nullptr;
    inline ImFont *montserrat_semibold = nullptr;
    inline ImFont *font_awesome = nullptr;
}// namespace fonts

namespace render
{
    void init( HWND hwnd, IDirect3DDevice9 *device );
    void show_objects( );
    void start( );
    void finish( );

    void string( ImFont *font, float x, float y, color col, const std::string &text, bool shadow = false, bool outline = false );
    void string( ImFont *font, glm::vec2 pos, color col, const std::string &text, bool shadow = false, bool outline = false );

    glm::vec2 get_text_size( ImFont *font, const std::string &text );

    void filled_rect( int x, int y, int w, int h, color col, float rounding = 0.f, int corner_flags = 0 );
    void filled_rect( glm::vec2 pos, glm::vec2 size, color col, float rounding = 0.f, int corner_flags = 0 );

    ImFont *create_from_system( const std::string &font_path, float size, const int flags, const ImWchar *glyph_ranges );
    ImFont *create_from_system( std::uint8_t *data, float size, size_t data_size, const int flags, const ImWchar *glyph_ranges );
    ImFont *create_from_system( std::uint8_t *data, float size, size_t data_size, const int flags, const ImWchar *glyph_ranges, ImFontConfig *cfg );

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
