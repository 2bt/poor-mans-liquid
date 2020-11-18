#include "simulation.hpp"
#include "fx.hpp"
#include <SDL_image.h>


class Game : public fx::App {
public:

    bool init() override {
        SDL_Surface* s = IMG_Load("world.png");
        if (!s) return false;

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

        for (int x = xx - 5; x < xx + 5; ++x)
        for (int y = yy - 5; y < yy + 5; ++y) {

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

        for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x) {
            if (m_sim.is_solid(x, y)) {
                fx::set_color(150, 120, 90);
                fx::draw_point(x, y);
            }
            int l = m_sim.get_liquid(x, y);
            if (l > 0) {
                fx::set_color(0, std::min(255, l * 50), 255, 100);
                fx::draw_point(x, y);
            }
        }


    }
    void key(int code) override {
        if (code == 40) init();
    }
private:
    Simulation m_sim;
};



int main() {
    Game game;
    return fx::run(game);
}
