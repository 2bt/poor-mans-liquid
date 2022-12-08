#pragma once
#include <vector>


class Simulation {
public:
    void init(int w, int h);
    void simulate();

    void set_solid(int x, int y, bool s) {
        if (!is_valid(x, y)) return;
        m_cells[x + y * m_width] = { s };
    }
    bool is_solid(int x, int y) const {
        return cell_at(x, y).solid;
    }

    void set_liquid(int x, int y, bool l) {
        if (!is_valid(x, y)) return;
        m_cells[x + y * m_width] = { false, l ? 1 : 0 };
    }
    int get_liquid(int x, int y) const {
        return cell_at(x, y).count;
    }

private:

    struct Cell {
        bool  solid = false;
        int   count = 0;
        float vx = 0;
        float vy = 0;

        int   d_count = 0;
        float d_vx = 0;
        float d_vy = 0;
    };

    Cell const& cell_at(int x, int y) const {
        if (!is_valid(x, y)) {
            static Cell c { true };
            return c;
        }
        return m_cells[y * m_width + x];
    }

    void apply_flow();
    void resolve_pressure();
    void apply_viscosity();



    bool is_valid(int x, int y) const {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }


    int               m_width;
    int               m_height;
    std::vector<Cell> m_cells;
};

