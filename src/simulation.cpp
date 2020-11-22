#include "simulation.hpp"
#include <random>
#include <algorithm>
#include <cmath>


namespace {

const std::array<int, 10>  offset_table = { -1, -1, -1, 0, 1, 1, 1, 0, -1, -1 };
std::default_random_engine random_engine(42);

int to_rand_int(float f) {
    static std::uniform_real_distribution<float> dist(0, 1);
    int i = std::floor(f);
    return i + (f - i > dist(random_engine));
}


} // namespace


void Simulation::init(int w, int h) {
    m_width  = w;
    m_height = h;
    m_cells.clear();
    m_cells.resize(m_width * m_height);
}

void Simulation::set_solid(int x, int y, bool s) {
    if (!is_valid(x, y)) return;
    m_cells[x + y * m_width] = { s };
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

        // gravity
        c.vy += 0.1 * c.count;

        // friction
        c.vx *= 0.99;
        c.vy *= 0.99;

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



void Simulation::get_random_neighbor(int& dx, int& dy) {
    if (m_neighbor_index_pos == 0) {
        std::shuffle(m_neighbor_indices.begin(), m_neighbor_indices.end(), random_engine);
    }
    int i = m_neighbor_indices[m_neighbor_index_pos];
    dy = offset_table[i];
    dx = offset_table[i + 2];
    m_neighbor_index_pos = (m_neighbor_index_pos + 1) % 8;
}


void Simulation::resolve_pressure() {
    for (int i = 0; i < 10; ++i) {

        for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x) {
            Cell& c = m_cells[x + y * m_width];
            if (c.count <= 1) continue;

            // find a random neighbor
            // transfer one unit of liquid (and force)
            for (int j = 0; j < 8; ++j) {
                int nx, ny;
                get_random_neighbor(nx, ny);
                float d = c.count * 0.7;
                int dx = nx * d;
                int dy = ny * d;

                if (is_solid(x + dx / 2, y + dy / 2)) continue;
                if (is_solid(x + dx, y + dy)) continue;

                Cell& n = m_cells[x + dx + (y + dy) * m_width];
                if (n.count <= c.count) {
                    float f = 0.5;
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
