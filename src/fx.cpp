#include "fx.hpp"
#include <SDL.h>
#include <array>
#include <cmath>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif


namespace fx {
namespace {

#include "font.hpp"

SDL_Window*    s_window;
SDL_Renderer*  s_renderer;
SDL_Texture*   s_font_tex;
Input          s_input;
bool           s_running;
int            s_result;
uint8_t const* s_keys;


class PixelTexture {
public:
    void free() {
        if (!m_tex) return;
        SDL_DestroyTexture(m_tex);
        m_tex = nullptr;
    }
    bool init(int w, int h) {
        m_width  = w;
        m_height = h;
        m_buffer.resize(m_width * m_height);
        m_tex = SDL_CreateTexture(s_renderer,
                                  SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  m_width, m_height);
        return !!m_tex;
    }
    void set_pixel(int x, int y, uint32_t color) {
        m_buffer[x + y * m_width] = color;
    }
    void draw() {
        SDL_UpdateTexture(m_tex, nullptr, m_buffer.data(), m_width * 4);
        SDL_RenderCopy(s_renderer, m_tex, nullptr, nullptr);
        std::fill(m_buffer.begin(), m_buffer.end(), 0);
    }
private:
    int                   m_width;
    int                   m_height;
    std::vector<uint32_t> m_buffer;
    SDL_Texture*          m_tex;
};


PixelTexture s_pixel_tex;



bool init_font() {
    std::array<uint16_t, 16 * 6 * 8 * 8> data;
    for (int i = 0; i < (int) data.size(); ++i) {
        data[i] = (FONT[i / 8] & (1 << (7 - i % 8))) ? 0xffff : 0;
    }
    s_font_tex = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_ARGB4444,
                                   SDL_TEXTUREACCESS_STATIC, 16 * 8, 6 * 8);
    if (!s_font_tex) return false;
    SDL_UpdateTexture(s_font_tex, nullptr, data.data(), 2 * 16 * 8);
    SDL_SetTextureBlendMode(s_font_tex, SDL_BLENDMODE_BLEND);
    return true;
}

void loop(void* arg) {
    App& app = *(App*) arg;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            s_running = false;
            break;

        case SDL_KEYDOWN:
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) s_running = false;
            app.key(e.key.keysym.scancode);
            break;

        case SDL_MOUSEBUTTONDOWN:
            app.click(e.button.button, e.button.x, e.button.y);
            break;

        default: break;
        }
    }


    s_input.prev_x = s_input.x;
    s_input.prev_y = s_input.y;
    s_input.prev_a = s_input.a;
    s_input.prev_b = s_input.b;
    s_input.x = !!s_keys[SDL_SCANCODE_RIGHT] - !!s_keys[SDL_SCANCODE_LEFT];
    s_input.y = !!s_keys[SDL_SCANCODE_DOWN] - !!s_keys[SDL_SCANCODE_UP];
    s_input.a = !!s_keys[SDL_SCANCODE_X];
    s_input.b = !!s_keys[SDL_SCANCODE_C];

    SDL_SetRenderDrawColor(s_renderer, 0, 0, 0, 0);
    SDL_RenderClear(s_renderer);

    app.update();

    SDL_RenderPresent(s_renderer);
}


} // namespace

void exit(int res) {
    s_result = res;
    s_running = false;
}


int run(App& app) {
    s_running = true;
    s_result = 0;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    s_window = SDL_CreateWindow("liquid",
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
#ifdef __EMSCRIPTEN__
                                WIDTH * 2, HEIGHT * 2, 0);
#else
                                WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);
#endif

    s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(s_renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(s_renderer, WIDTH, HEIGHT);
    s_keys = SDL_GetKeyboardState(nullptr);

    if (!init_font()) {
        LOG_ERROR("font_init failed");
        s_running = false;
        s_result = 1;
    }
    if (!s_pixel_tex.init(WIDTH, HEIGHT)) {
        LOG_ERROR("pixel_tex.init failed");
        s_running = false;
        s_result = 1;
    }
    if (!app.init()) {
        LOG_ERROR("app.init failed");
        s_running = false;
        s_result = 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(loop, &app, -1, 1);
#else
    while (s_running) loop(&app);
#endif

    app.free();
    SDL_DestroyTexture(s_font_tex);
    SDL_DestroyRenderer(s_renderer);
    SDL_DestroyWindow(s_window);
    SDL_Quit();
    return s_result;
}

Input const& input() { return s_input; }

bool key_state(int code) {
    return !!s_keys[code];
}

void set_color(int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(s_renderer, r, g, b, a);
}

void draw_line(float x1, float y1, float x2, float y2) {
    SDL_RenderDrawLine(s_renderer, (int) std::floor(x1), (int) std::floor(y1), (int) std::floor(x2), (int) std::floor(y2));
}

void draw_rectangle(bool fill, Rect const& rect) {
    SDL_Rect r = { rect.x, rect.y, rect.w, rect.h };
    if (fill) SDL_RenderFillRect(s_renderer, &r);
    else      SDL_RenderDrawRect(s_renderer, &r);
}
void draw_rectangle(bool fill, float x, float y, float w, float h) {
    draw_rectangle(fill, { (int) std::floor(x), (int) std::floor(y), (int) std::floor(w), (int) std::floor(h) });
}

void draw_point(float x, float y) {
    SDL_RenderDrawPointF(s_renderer, x, y);
}

void set_font_color(int r, int g, int b) {
    SDL_SetTextureColorMod(s_font_tex, r, g, b);
}

void put_char(float x, float y, char c) {
    if (c < 32) return;
    SDL_Rect src = { c % 16 * 8, (c - 32) / 16 * 8, 8, 8 };
    SDL_Rect dst = { (int) std::floor(x), (int) std::floor(y), 8, 8 };
    SDL_RenderCopy(s_renderer, s_font_tex, &src, &dst);
}

void print(float x, float y, const char* str) {
    while (*str) {
        put_char(x, y, *str);
        ++str;
        x += 8;
    }
}

void printf(float x, float y, const char* format, ...) {
    va_list args;
    va_start(args, format);
    static std::array<char, 1024> line;
    vsnprintf(line.data(), line.size(), format, args);
    va_end(args);
    print(x, y, line.data());
}


void set_pixel(int x, int y, int r, int g, int b) {
    s_pixel_tex.set_pixel(x, y, r << 16 | g << 8 | b << 0);
}
void draw_pixels() {
    s_pixel_tex.draw();
}

} // namespace
