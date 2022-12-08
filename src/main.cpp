#include "simulation.hpp"
#include "fx.hpp"
#include <string>
#include <chrono>
#include <SDL_image.h>
#include <SDL.h>


class Game : public fx::App {
public:

    bool init() override {
        std::string filename = "./scenes/" + std::to_string(m_scene) + ".png";
        SDL_Surface* img = IMG_Load(filename.c_str());
        if (!img) {
            printf("error: cannot open %s\n", filename.c_str());
            return false;
        }
        m_sim.init(img->w, img->h);
        for (int y = 0; y < img->h; ++y)
        for (int x = 0; x < img->w; ++x) {
            uint8_t* a = (uint8_t*) img->pixels + y * img->pitch + x * 3;
            uint32_t p = (a[0] << 0) | (a[1] << 8) | (a[2] << 16);
            if (p == 0xffffff) m_sim.set_solid(x, y, true);
            if (p == 0xff0000) m_sim.set_liquid(x, y, true);
        }
        SDL_FreeSurface(img);
        return true;
    }

    void update() override {

        spawn();

        // similate & track time
        auto start = std::chrono::high_resolution_clock::now();
        m_sim.simulate();
        auto end = std::chrono::high_resolution_clock::now();
        m_time_sum     += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        m_time_counter += 1;
        if (m_time_counter >= 60) {
            m_time         = m_time_sum / m_time_counter;
            m_time_sum     = 0;
            m_time_counter = 0;
        }


        // screenshot
        if (m_recording && m_frame_nr < 60 * 7) {
            m_screenshot = SDL_CreateRGBSurfaceWithFormat(0, WIDTH, HEIGHT, 8, SDL_PIXELFORMAT_RGB24);
        }

        // draw scene
        for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x) {
            if (m_sim.get_liquid(x, y)) pixel(x, y, 0, 0, 255);
            if (m_sim.is_solid(x, y))   pixel(x, y, 100, 100, 100);
        }
        fx::draw_pixels();
        fx::printf(4, 4, "TIME:%6d", m_time);


        if (m_screenshot) {
            static char name[64];
            sprintf(name, "%04d.png", m_frame_nr++);
            IMG_SavePNG(m_screenshot, name);
            SDL_FreeSurface(m_screenshot);
            m_screenshot = nullptr;
        }
    }
    void key(int code) override {
        if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_7) {
            m_scene = code - SDL_SCANCODE_1 + 1;
            m_frame_nr = 0;
            init();
        }
    }
    void mouse_click(int button, bool state, int x, int y) override {
        if (!state) {
            m_spawn_enabled = false;
            return;
        }
        m_spawn_x = x;
        m_spawn_y = y;
        if (button == 1) {
            m_spawn_enabled = true;
            m_spawn_erase   = false;
        }
        else if (button == 3) {
            m_spawn_enabled = true;
            m_spawn_erase   = true;
        }
    }
    void mouse_move(uint32_t state, int x, int y) override {
        m_spawn_x = x;
        m_spawn_y = y;
    }

    void spawn() {
        if (!m_spawn_enabled) return;
        bool solid = fx::key_state(SDL_SCANCODE_LSHIFT) || fx::key_state(SDL_SCANCODE_RSHIFT);
        int r = solid ? 3 : 8;
        for (int dy = -r; dy <= r; ++dy)
        for (int dx = -r; dx <= r; ++dx) {
            if (dx * dx + dy * dy > r * r + 3) continue;
            int tx = m_spawn_x + dx;
            int ty = m_spawn_y + dy;
            if (solid) {
                m_sim.set_solid(tx, ty, !m_spawn_erase);
            }
            else {
                if (!m_sim.is_solid(tx, ty)) m_sim.set_liquid(tx, ty, !m_spawn_erase);
            }
        }
    }

    void pixel(int x, int y, int r, int g, int b) {
        fx::set_pixel(x, y, r, g, b);
        if (m_screenshot) {
            uint8_t* a = (uint8_t*) m_screenshot->pixels + y * m_screenshot->pitch + x * 3;
            a[0] = r;
            a[1] = g;
            a[2] = b;
        }
    }

    void set_recording(bool r) { m_recording = r; }

private:
    int          m_scene = 1;
    Simulation   m_sim;

    int          m_time_counter = 0;
    int          m_time_sum     = 0;
    int          m_time         = 0;

    int          m_spawn_x;
    int          m_spawn_y;
    bool         m_spawn_enabled = false;
    bool         m_spawn_erase   = false;

    bool         m_recording  = false;
    int          m_frame_nr   = 0;
    SDL_Surface* m_screenshot = nullptr;
};



int main(int argc, char** argv) {
    Game game;
    if (argc == 2 && std::string(argv[1]) == "--record") {
        game.set_recording(true);
    }
    return fx::run(game);
}
