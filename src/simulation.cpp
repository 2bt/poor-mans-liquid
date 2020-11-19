#include "simulation.hpp"
#include <array>
#include <random>
#include <algorithm>
#include <cmath>


namespace {

float mix(float x, float y, float a) {
    return x * (1 - a) + y * a;
}


std::default_random_engine random_engine(42);


int to_rand_int(float f) {
    static std::uniform_real_distribution<float> dist(0, 1);
    int i = floor(f);
    return i + (f - i > dist(random_engine));
}


} // namespace


void Simulation::init(int w, int h) {
    m_width  = w;
    m_height = h;
    m_cells.clear();
    m_cells.resize(m_width * m_height);
}

void Simulation::set_solid(int x, int y) {
    if (!is_valid(x, y)) return;
    m_cells[x + y * m_width] = { true };
}
bool Simulation::is_solid(int x, int y) const {
    return cell_at(x, y).solid;
}


void Simulation::set_liquid(int x, int y, int l) {
    if (!is_valid(x, y)) return;
    m_cells[x + y * m_width] = { false, l };
}
int Simulation::get_liquid(int x, int y) const {
    return cell_at(x, y).count;
}


void Simulation::simulate() {

    for (int i = 0; i < 1; ++i) {

        // apply gravity
        for (Cell& c : m_cells) {
            if (c.count > 0) c.vy += 0.1 * c.count;
            c.d_count = 0;
            c.d_vx = 0;
            c.d_vy = 0;
        }

        apply_flow();

        resolve_pressure();
        apply_viscosity();
        resolve_pressure();
        apply_viscosity();
    }
}



void Simulation::apply_flow() {

    for (int y = 0; y < m_height; ++y)
    for (int x = 0; x < m_width; ++x) {
        Cell& c = m_cells[x + y * m_width];

        for (int i = 0; i < c.count; ++i) {

            int dx = to_rand_int(c.vx);
            int dy = to_rand_int(c.vy);

            // collision

            // don't go through walls too much
            if (is_solid(x + dx / 2, y + dy / 2)) {
                dx /= 3;
                dy /= 3;
            }

            if (is_solid(x + dx, y)) {
                dx   = 0;
                c.vx = 0;
            }

            if (is_solid(x + dx, y + dy)) {
                dy   = 0;
                c.vy = 0;
            }

            Cell& dst = m_cells[x + dx + (y + dy) * m_width];

            dst.d_count += 1;
            dst.d_vx    += c.vx / c.count;
            dst.d_vy    += c.vy / c.count;
        }
    }

    for (Cell& c : m_cells) {
        c.count = c.d_count;
        c.vx    = c.d_vx;
        c.vy    = c.d_vy;
        c.d_count = 0;
        c.d_vx    = 0;
        c.d_vy    = 0;
    }
}


void Simulation::resolve_pressure() {
    // resolve pressure
    for (int i = 0; i < 10; ++i) {

        for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x) {
            Cell& c = m_cells[x + y * m_width];
            if (c.count <= 1) continue;

            // find a random neighbor and transfer one unit
            static std::array<int, 8> dirs = { 0, 1, 2, 3, 4, 5, 6, 7 };
            std::shuffle(dirs.begin(), dirs.end(), random_engine);

            for (int d : dirs) {
                static std::array<int, 10> deltas = { -1, -1, -1, 0, 1, 1, 1, 0, -1, -1 };
                int dy = deltas[d    ] * c.count * 0.8;
                int dx = deltas[d + 2] * c.count * 0.8;

                if (is_solid(x + dx / 2, y + dy / 2)) continue;

                if (is_solid(x + dx, y + dy)) continue;
                Cell& n = m_cells[x + dx + (y + dy) * m_width];
                if (n.count < c.count) {
                    float f = 0.6;
                    float g = 0.2;
                    n.d_vx    += c.vx / c.count + dx * f;
                    n.d_vy    += c.vy / c.count + dy * f;
                    n.d_count += 1;

                    c.d_vx    -= c.vx / c.count + dx * g;
                    c.d_vy    -= c.vy / c.count + dy * g;
                    c.d_count -= 1;
                    break;
                }
            }

        }

        for (Cell& c : m_cells) {
            c.count += c.d_count;
            c.vx    += c.d_vx;
            c.vy    += c.d_vy;
            c.d_count = 0;
            c.d_vx    = 0;
            c.d_vy    = 0;
        }

    }
}


void Simulation::apply_viscosity() {

    for (int y = 0; y < m_height; ++y)
    for (int x = 0; x < m_width; ++x) {
        Cell& c = m_cells[x + y * m_width];
        if (c.count == 0) continue;

        Cell const& n1 = cell_at(x-1, y-1);
        Cell const& n2 = cell_at(x-1, y+1);
        Cell const& n3 = cell_at(x+1, y-1);
        Cell const& n4 = cell_at(x+1, y+1);

        float vx = n1.vx + n2.vx + n3.vx + n4.vx;
        float vy = n1.vy + n2.vy + n3.vy + n4.vy;
        int count = n1.count + n2.count + n3.count + n4.count;
        if (count == 0) {
            c.d_vx = c.vx;
            c.d_vy = c.vy;
            continue;
        }

        c.d_vx = c.count * mix(c.vx / c.count, vx / count, 0.7);
        c.d_vy = c.count * mix(c.vy / c.count, vy / count, 0.7);

    }
    for (Cell& c : m_cells) {
        c.vx   = c.d_vx;
        c.vy   = c.d_vy;
        c.d_vx = 0;
        c.d_vy = 0;
    }
}
