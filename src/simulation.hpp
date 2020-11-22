#pragma once

#include <array>
#include <vector>


class Simulation {
public:
    void init(int w, int h);
    void simulate();

    void set_solid(int x, int y, bool s = true);
    bool is_solid(int x, int y) const;

    void set_liquid(int x, int y, int l);
    int  get_liquid(int x, int y) const;

private:
    void apply_flow();
    void resolve_pressure();
    void apply_viscosity();

    struct Cell {
        bool  solid = false;
        int   count = 0;
        float vx = 0;
        float vy = 0;

        int   d_count = 0;
        float d_vx = 0;
        float d_vy = 0;
    };


    bool is_valid(int x, int y) const {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }

    void get_random_neighbor(int& dx, int& dy);

    Cell const& cell_at(int x, int y) const {
        if (!is_valid(x, y)) {
            static Cell c { true };
            return c;
        }
        return m_cells[y * m_width + x];
    }

    int               m_width;
    int               m_height;
    std::vector<Cell> m_cells;


    std::array<int, 8> m_neighbor_indices = { 0, 1, 2, 3, 4, 5, 6, 7 };
    int                m_neighbor_index_pos = 0;
};

