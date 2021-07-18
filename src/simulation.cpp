#include "simulation.hpp"
#include <random>
#include <algorithm>
#include <cmath>


namespace {

std::default_random_engine random_engine(42);

int rand_int(int a, int b) {
    return std::uniform_int_distribution(a, b)(random_engine);
}

int to_rand_int(float f) {
    static std::uniform_real_distribution<float> dist(0, 1);
    int i = std::floor(f);
    return i + (f - i > dist(random_engine));
}



struct Offset { int dx, dy; };

std::vector<Offset> m_random_offsets;
std::array<int, 7>  m_random_offset_indices = {};
enum { OFFSET_COUNT = 1 << 16 };

Offset get_random_offset(int i) {
    i = std::min<int>(i, m_random_offset_indices.size() - 1);
    int& index = m_random_offset_indices[i];
    index = (index + 1) % OFFSET_COUNT;
    return m_random_offsets[i * OFFSET_COUNT + index];
}



} // namespace


void Simulation::init(int w, int h) {
    m_width  = w;
    m_height = h;
    m_cells.clear();
    m_cells.resize(m_width * m_height);

    // init offset table
    for (int r = 1; r <= (int) m_random_offset_indices.size(); ++r) {
        for (int i = 0; i < OFFSET_COUNT; ++i) {
            int dx, dy;
            for (;;) {
                dx = rand_int(-r, r);
                dy = rand_int(-r, r);
                int l = dx * dx + dy * dy;
                if (l == 0 || l > r * r + 2) continue;
                break;
            }
            m_random_offsets.push_back({ dx, dy });
        }
    }
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

        // gravity
        c.vy += 0.1f * c.count;

        // friction
        c.vx *= 0.99f;
        c.vy *= 0.99f;

        for (int i = 0; i < c.count; ++i) {

            int dx = to_rand_int(c.vx);
            int dy = to_rand_int(c.vy);

            // collision

            // don't go through walls too much
            if (is_solid(x + dx / 2, y + dy / 2)) {
                dx /= 2;
                dy /= 2;
            }

            if (is_solid(x + dx, y)) {
                dx   = 0;
                c.vx = 0;
            }

            if (is_solid(x + dx, y + dy)) {
                dy   = 0;
                c.vy = 0;
            }

            int tx = x + dx;
            int ty = y + dy;
            Cell& dst = m_cells[tx + ty * m_width];

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
    for (int i = 0; i < 10; ++i) {

        for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x) {
            Cell& c = m_cells[x + y * m_width];
            if (c.count < 2) continue;

            // find a random neighbor
            // transfer liquid

            for (int j = 0; j < 8; ++j) {

                auto o = get_random_offset(c.count - 2);
                int dx = o.dx;
                int dy = o.dy;

                if (is_solid(x + dx / 2, y + dy / 2)) continue;
                if (is_solid(x + dx, y + dy)) continue;

                Cell& n = m_cells[x + dx + (y + dy) * m_width];

                if (c.count >= n.count) {
                    float f = 0.5f;
                    n.d_vx    += c.vx / c.count + dx * f;
                    n.d_vy    += c.vy / c.count + dy * f;
                    n.d_count += 1;
                    c.d_vx    -= c.vx / c.count;
                    c.d_vy    -= c.vy / c.count;
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
        int weight = 1;
        float vx    = c.vx * weight;
        float vy    = c.vy * weight;
        int   count = c.count * weight;
        for (int i = 0; i < 8; ++i) {
            static const std::array<int, 10> offset_table = { -1, -1, -1, 0, 1, 1, 1, 0, -1, -1 };
            Cell const& n = cell_at(x + offset_table[i], y + offset_table[i + 2]);
            vx    += n.vx;
            vy    += n.vy;
            count += n.count;
        }
        c.d_vx = vx / count * c.count;
        c.d_vy = vy / count * c.count;
    }
    for (Cell& c : m_cells) {
        c.vx   = c.d_vx;
        c.vy   = c.d_vy;
        c.d_vx = 0;
        c.d_vy = 0;
    }
}
