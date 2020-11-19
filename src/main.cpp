#include "simulation.hpp"
#include "fx.hpp"
#include <string>
#include <SDL_image.h>
#include <SDL.h>


class Game : public fx::App {
public:

    bool init() override {
        std::string filename = "./scenes/" + std::to_string(m_scene) + ".png";
        SDL_Surface* s = IMG_Load(filename.c_str());
        if (!s) {
            printf("error: cannot open %s\n", filename.c_str());
            return false;
        }

        m_sim.init(s->w, s->h);

        for (int y = 0; y < s->h; ++y)
        for (int x = 0; x < s->w; ++x) {
            uint8_t* a = (uint8_t*) s->pixels + y * s->pitch + x * 3;
            uint32_t p = (a[0] << 0) | (a[1] << 8) | (a[2] << 16);
            if (p == 0xffffff) {
                m_sim.set_solid(x, y);
            }
            if (p == 0xff0000) {
                m_sim.set_liquid(x, y, 1);
            }
        }
        SDL_FreeSurface(s);
        return true;
    }

    void click(int button, int xx, int yy) override {

        for (int x = xx - 15; x < xx + 15; ++x)
        for (int y = yy - 15; y < yy + 15; ++y) {

            // spawn more liquid
            if (button == 1) {
                if (!m_sim.is_solid(x, y)) m_sim.set_liquid(x, y, 1);
            }

            // erase
            if (button == 3) {
                if (!m_sim.is_solid(x, y)) m_sim.set_liquid(x, y, 0);
            }

        }
    }


    void update() override {

        m_sim.simulate();

        // draw scene
        for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x) {
            int l = 0;
            l += m_sim.get_liquid(x, y) * 4;
            l += m_sim.get_liquid(x + 1, y) * 2;
            l += m_sim.get_liquid(x - 1, y) * 2;
            l += m_sim.get_liquid(x, y + 1) * 2;
            l += m_sim.get_liquid(x, y - 1) * 2;
            l += m_sim.get_liquid(x + 1, y - 1);
            l += m_sim.get_liquid(x + 1, y + 1);
            l += m_sim.get_liquid(x - 1, y - 1);
            l += m_sim.get_liquid(x - 1, y + 1);
            if (l > 2) {
                fx::set_color(0, l, 100);
                fx::draw_point(x, y);
            }

            if (m_sim.is_solid(x, y)) {
                float w = std::sin(x) * std::sin(y);
                fx::set_color(100 + w * 30, 80 + w * 10, 50 - w * 20);
                fx::draw_point(x, y);
            }
        }

        fx::set_font_color(255, 255, 255);
        fx::printf(8, 8, "scene: %d", m_scene);

    }
    void key(int code) override {
        if (code == SDL_SCANCODE_RETURN) init();
        if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_5) {
            m_scene = code - SDL_SCANCODE_1 + 1;
            init();
        }
    }
private:
    int        m_scene = 1;
    Simulation m_sim;
};



int main() {
    Game game;
    return fx::run(game);
}
