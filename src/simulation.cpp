#include "simulation.hpp"
#include <random>
#include <algorithm>
#include <cmath>


namespace {


std::default_random_engine random_engine(42);

//int rand_int(int a, int b) {
//    return std::uniform_int_distribution(a, b)(random_engine);
//}

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

    apply_flow();

    for (int j = 0; j < 4; ++j) {
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
    static const std::array<int, 10> table = { -1, -1, -1, 0, 1, 1, 1, 0, -1, -1 };
    if (m_neighbor_index_pos == 0) {
        std::shuffle(m_neighbor_indices.begin(), m_neighbor_indices.end(), random_engine);
    }
    dy = table[m_neighbor_index_pos];
    dx = table[m_neighbor_index_pos + 2];
    m_neighbor_index_pos = (m_neighbor_index_pos + 1) & 7;
}


void Simulation::resolve_pressure() {

    for (int i = 0; i < 10; ++i) {


        for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x) {
            Cell& c = m_cells[x + y * m_width];
            if (c.count <= 1) continue;

            // find a random neighbor and transfer one unit
            for (int j = 0; j < 8; ++j) {
                int dx, dy;
                get_random_neighbor(dx, dy);
                dy *= c.count * 0.8;
                dx *= c.count * 0.8;

                if (is_solid(x + dx / 2, y + dy / 2)) continue;

                if (is_solid(x + dx, y + dy)) continue;
                Cell& n = m_cells[x + dx + (y + dy) * m_width];
                if (n.count < c.count + 1) {
                    float f = 0.6;
                    float g = 0.4;
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

        int weight = 3;

        float vx  = c.vx * weight;
        float vy  = c.vy * weight;
        int count = c.count * weight;
        vx    += n1.vx + n2.vx + n3.vx + n4.vx;
        vy    += n1.vy + n2.vy + n3.vy + n4.vy;
        count += n1.count + n2.count + n3.count + n4.count;

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
