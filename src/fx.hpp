#pragma once

#include <cstdio>


#define LOG_ERROR(fmt, ...) fprintf(stderr, "ERROR - " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  fprintf(stderr, "WARN  - " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  fprintf(stderr, "INFO  - " fmt "\n", ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) fprintf(stderr, "DEBUG - " fmt "\n", ##__VA_ARGS__)


enum {
    WIDTH     = 384,
    HEIGHT    = 216,
};


namespace fx {

    struct App {
        virtual ~App() {}
        virtual bool init() { return true; }
        virtual void free() {}
        virtual void key(int code) {}
        virtual void click(int button, int x, int y) {}
        virtual void update() = 0;
    };
    struct Rect {
        int x, y, w, h;
    };
    struct Input {
        int  x, y, prev_x, prev_y;
        bool a, b, prev_a, prev_b;
    };

    void set_color(int r, int g, int b, int a = 255);
    void draw_line(float x1, float y1, float x2, float y2);
    void draw_rectangle(bool fill, Rect const& rect);
    void draw_rectangle(bool fill, float x, float y, float w, float h);
    void draw_point(float x, float y);
    void set_font_color(int r, int g, int b);
    void put_char(float x, float y, char c);
    void print(float x, float y, const char* str);
    void printf(float x, float y, const char* format, ...);

    void set_pixel(int x, int y, int r, int g, int b);
    void draw_pixels();

    Input const& input();
    bool key_state(int code);
    void exit(int result = 0);
    int run(App& App);
}
