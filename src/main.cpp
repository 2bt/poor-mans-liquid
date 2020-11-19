#include "simulation.hpp"
#include "fx.hpp"
#include <string>
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
            if (p == 0xffffff) {
                m_sim.set_solid(x, y);
            }
            if (p == 0xff0000) {
                m_sim.set_liquid(x, y, 1);
            }
        }
        SDL_FreeSurface(img);
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


    SDL_Surface* m_img      = nullptr;
    int          m_frame_nr = 0;

    void draw_pixel(int x, int y, int r, int g, int b) {
        fx::set_color(r, g, b);
        fx::draw_point(x, y);
        if (m_img) {
            uint8_t* a = (uint8_t*) m_img->pixels + y * m_img->pitch + x * 3;
            a[0] = r;
            a[1] = g;
            a[2] = b;
        }
    }

    void update() override {

        m_sim.simulate();

        // screenshot
//        if (m_frame_nr < 60 * 5) {
//            m_img = SDL_CreateRGBSurfaceWithFormat(0, WIDTH, HEIGHT, 8, SDL_PIXELFORMAT_RGB24);
//        }

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
                draw_pixel(x, y, 0, 0, 150);
            }

            if (m_sim.is_solid(x, y)) {
                int s = 0;
                s += m_sim.is_solid(x + 1, y);
                s += m_sim.is_solid(x - 1, y);
                s += m_sim.is_solid(x, y + 1);
                s += m_sim.is_solid(x, y - 1);
                float f = s == 4 ? 1 : 0.6;
                draw_pixel(x, y, 100 * f, 80 * f, 50 * f);
            }
        }


        if (m_img) {
            static char name[64];
            sprintf(name, "%04d.png", m_frame_nr++);
            IMG_SavePNG(m_img, name);
            SDL_FreeSurface(m_img);
            m_img = nullptr;
        }
    }
    void key(int code) override {
        if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_7) {
            m_scene = code - SDL_SCANCODE_1 + 1;
            m_frame_nr = 0;
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
