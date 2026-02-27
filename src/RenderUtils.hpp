#pragma once

#include "libfixmath/fix16.hpp"

// TODO: Only needed for structs fix16_vec3. Move these somewhere else...
#include "Fix16_Utils.hpp"
#include <cstdlib> // for abs

#ifdef PC
    typedef uint32_t color_t; // SDL2 uses 32b colors (24b colors + 8b alpha). Alpha not used.
#else
    typedef uint16_t color_t; // ClassPad uses 16b colors
    extern uint16_t* global_vram;
    extern uint16_t* backbuffer;
    extern unsigned int screen_width;
    extern unsigned int screen_height;
#endif

// Inline Graphics Helpers
#ifndef PC
inline color_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

inline void setPixel(int x, int y, color_t c) {
    if (x >= 0 && x < (int)screen_width && y >= 0 && y < (int)screen_height) {
        backbuffer[y * screen_width + x] = c;
    }
}

// Bresenham's line algorithm
inline void line(int x0, int y0, int x1, int y1, color_t c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;) {
        setPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
#else
// PC implementations usually in RenderUtils.cpp or PC_SDL_screen.hpp
// Assuming they are defined there or we need to add prototypes here if used in headers?
// RenderUtils.cpp uses them, but if they are functions, they need prototypes.
// In legacy code, they were likely macros or functions.
// Let's add prototypes for PC build if they aren't there.
// Actually, PC build uses SDL, so these might be implemented differently.
// Let's check RenderUtils.cpp
color_t rgb565(uint8_t r, uint8_t g, uint8_t b);
#define color(r,g,b) rgb565(r,g,b)
void setPixel(int x, int y, color_t c);
void line(int x0, int y0, int x1, int y1, color_t c);
#endif

Fix16 calculateLightIntensityPointLight(const fix16_vec3& lightPos,     const fix16_vec3& surfacePos, const fix16_vec3& normal, Fix16 lightIntensity);
Fix16 calculateLightIntensityDirLight  (const fix16_vec3& lightNormDir, const fix16_vec3& surfaceNorm, Fix16 lightIntensity);

void drawHorizontalLine(
    int x0, int x1, int y,
    int u0, int u1, int v0, int v1,
    uint32_t *texture, int textureWidth, int textureHeight,
    Fix16 lightInstensity = 1.0f
);

void drawTriangle(
    int16_t_Point2d v0, int16_t_Point2d v1, int16_t_Point2d v2,
    uint32_t *texture, int textureWidth, int textureHeight,
    Fix16 lightInstensity = 1.0f
);
void draw_center_square(int16_t cx, int16_t cy, int16_t sx, int16_t sy, color_t color);

void draw_RotationVisualizer(fix16_vec2 camera_rot);

template <class T>
inline void swap(T& a, T& b) {
    T tmp = b;
    b = a;
    a = tmp;
}

void bubble_sort(uint_fix16_t a[], int n);
