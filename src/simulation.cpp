#include "simulation.hpp"
#include <random>
#include <algorithm>
#include <cmath>


namespace {

std::default_random_engine g_random_engine(42);

int to_rand_int(float f) {
    static std::uniform_real_distribution<float> dist(0, 1);
    int i = std::floor(f);
    return i + (f - i > dist(g_random_engine));
}


struct Offset { int dx, dy; };
Offset get_random_offset() {
    std::array<Offset, 32> OFFSETS = {
        Offset{-1, 1},
        Offset{-1, 1},
        Offset{ 1,-1},
        Offset{-1, 0},
        Offset{ 1, 0},
        Offset{ 1, 1},
        Offset{ 0, 1},
        Offset{-1,-1},
        Offset{ 1, 0},
        Offset{ 0,-1},
        Offset{ 0, 1},
        Offset{ 1,-1},
        Offset{ 1, 1},
        Offset{-1, 1},
        Offset{ 1, 1},
        Offset{ 0, 1},
        Offset{ 0,-1},
        Offset{ 0, 1},
        Offset{-1, 0},
        Offset{ 0,-1},
        Offset{ 1,-1},
        Offset{ 1, 1},
        Offset{ 0,-1},
        Offset{-1,-1},
        Offset{-1, 1},
        Offset{-1,-1},
        Offset{-1, 0},
        Offset{ 1, 0},
        Offset{-1,-1},
        Offset{ 1,-1},
        Offset{ 1, 0},
        Offset{-1, 0},
    };
    static int index = 0;
    index = (index + 1) % OFFSETS.size();
    return OFFSETS[index];
}


} // namespace


void Simulation::init(int w, int h) {
    m_width  = w;
    m_height = h;
    m_cells.clear();
    m_cells.resize(m_width * m_height);
}


void Simulation::simulate() {
    apply_flow();
    for (int j = 0; j < 2; ++j) {
        resolve_pressure();
        apply_viscosity();
    }
}


void Simulation::apply_flow() {

    for (int y = 0; y < m_height; ++y)
    for (int x = 0; x < m_width; ++x) {
        Cell& c = m_cells[x + y * m_width];
        if (c.count == 0) continue;

        // friction && gravity
        float vx = c.vx * 0.99f;
        float vy = c.vy * 0.99f + 0.1f; // * c.count;

        int dx = to_rand_int(vx);
        int dy = to_rand_int(vy);

        // collision
        // don't go through walls too much
        if (is_solid(x + dx / 2, y + dy / 2)) {
            dx /= 2;
            dy /= 2;
        }
        if (is_solid(x + dx, y)) {
            dx = 0;
            vx = 0;
        }
        if (is_solid(x + dx, y + dy)) {
            dy = 0;
            vy = 0;
        }

        int tx = x + dx;
        int ty = y + dy;
        Cell& dst = m_cells[tx + ty * m_width];

        dst.d_count += c.count;
        dst.d_vx    += vx;
        dst.d_vy    += vy;
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
    for (int i = 0; i < 4; ++i) {

        for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x) {
            Cell& c = m_cells[x + y * m_width];

            for (int j = 0; j < c.count - 1; ++j) {

                // find a random neighbor
                // transfer liquid
                Offset o = get_random_offset();
                int dx = o.dx;
                int dy = o.dy;

                if (is_solid(x + dx / 2, y + dy / 2)) continue;
                if (is_solid(x + dx, y + dy)) continue;

                float const BUBBLINESS = 0.7f;

                Cell& n = m_cells[x + dx + (y + dy) * m_width];
                n.d_vx    += c.vx / c.count + dx * BUBBLINESS;
                n.d_vy    += c.vy / c.count + dy * BUBBLINESS;
                n.d_count += 1;
                c.d_vx    -= c.vx / c.count;
                c.d_vy    -= c.vy / c.count;
                c.d_count -= 1;
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
        float vx    = 0;
        float vy    = 0;
        float count = 0;
        int const S = 1;
        for (int ox = -S; ox <= S; ++ox)
        for (int oy = -S; oy <= S; ++oy) {
            Cell const& n = cell_at(x + ox, y + oy);
            vx    += n.vx;
            vy    += n.vy;
            count += n.count;
        }
        c.d_vx = vx * (c.count / count);
        c.d_vy = vy * (c.count / count);
    }
    for (Cell& c : m_cells) {
        c.vx   = c.d_vx;
        c.vy   = c.d_vy;
        c.d_vx = 0;
        c.d_vy = 0;
    }
}
