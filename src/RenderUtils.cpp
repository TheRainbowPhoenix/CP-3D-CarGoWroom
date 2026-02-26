#include "RenderUtils.hpp"

#include "Fix16_Utils.hpp"

#include "RenderFP3D.hpp"

#ifndef PC
#   include <sdk/os/lcd.h>
    // Global VRAM pointers
    uint16_t* global_vram;
    int screen_width, screen_height;
#else
#   include "PC_SDL_screen.hpp" // replaces "sdk/os/lcd.hpp"
#endif

// Light intensity range 1.0f - MIN_LIGHT_INTENSITY
// It looks much better if colors wont go to full black
#define MIN_LIGHT_INTENSITY 0.30f

#ifdef PC
// PC Implementations of graphics primitives
color_t color(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

// setPixel for PC is likely handled via setPixel_Unsafe in PC_SDL_screen.hpp or similar
// But we need to implement the function if it's called.
// Checking PC_SDL_screen.hpp would be good, but for now implementing a wrapper
// assuming setPixel_Unsafe is available or we use SDL directly.
// Wait, RenderUtils.cpp included "PC_SDL_screen.hpp".
// Let's implement it using setPixel_Unsafe if available, or just a dummy if PC build isn't primary focus.
// However, the error log showed "setPixel was not declared in this scope" for non-PC build too?
// No, the error was "src/RenderUtils.cpp:96:13: error: 'setPixel' was not declared in this scope".
// This was likely for non-PC build because I hadn't defined it yet.
// For PC build, I should ensure it exists.
// Let's implement a simple version or rely on the header.
void setPixel(int x, int y, color_t c) {
    setPixel_Unsafe(x, y, c);
}

void line(int x0, int y0, int x1, int y1, color_t c) {
    // Basic Bresenham for PC if needed, or use SDL
    // Since PC_SDL_screen.hpp might have it?
    // Let's just implement it to be safe.
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
#endif

// -- Some Helper functions
// -- TODO: Move these somewhere else
Fix16 calculateLightIntensityPointLight(const fix16_vec3& lightPos, const fix16_vec3& surfacePos, const fix16_vec3& normal, Fix16 lightIntensity)
{
    fix16_vec3 lightDir = {lightPos.x - surfacePos.x, lightPos.y - surfacePos.y, lightPos.z - surfacePos.z};
    normalize_fix16_vec3(lightDir);

    // Intensity from 0.0f -> 1.0f
    Fix16 intensity = lightIntensity * fix16_max(0, lightDir.x * normal.x + lightDir.y * normal.y + lightDir.z * normal.z);

    // Ensure intensity is at least minIntensity
    intensity = fix16_max(intensity, Fix16(MIN_LIGHT_INTENSITY));

    // Mapping between target value
    return intensity;
}

Fix16 calculateLightIntensityDirLight  (const fix16_vec3& lightNormDir, const fix16_vec3& surfaceNorm, Fix16 lightIntensity)
{
    // Calculate the dot product between the light direction and the surface normal
    Fix16 dotProduct = lightNormDir.x * surfaceNorm.x + lightNormDir.y * surfaceNorm.y + lightNormDir.z * surfaceNorm.z;

    // Intensity
    Fix16 intensity = lightIntensity * dotProduct;

    // Ensure intensity is at least minIntensity
    intensity = fix16_max(intensity, Fix16(MIN_LIGHT_INTENSITY));

    // Mapping between target value
    return intensity;
}

void drawHorizontalLine(
    int x0, int x1, int y,
    int u0, int u1, int v0, int v1,
    uint32_t *texture, int textureWidth, int textureHeight,
    Fix16 lightInstensity
) {
    if (x0 > x1) {
        swap(x0, x1);
        swap(u0, u1);
        swap(v0, v1);
    }

    if (x0 == x1) {
        int u = u0;
        int v = v0;

        if (u >= 0 && u < textureWidth && v >= 0 && v < textureHeight) {
            auto texel = texture[u + v * textureWidth];
            uint8_t r = (0xff & (texel>>16));
            uint8_t g = (0xff & (texel>>8));
            uint8_t b = (0xff & texel);
            r = (uint8_t) ((int16_t)(Fix16((int16_t)r) * lightInstensity));
            g = (uint8_t) ((int16_t)(Fix16((int16_t)g) * lightInstensity));
            b = (uint8_t) ((int16_t)(Fix16((int16_t)b) * lightInstensity));
            auto c = color(r,g,b);
            setPixel(x0, y, c);
        }
        return;
    }

    for (int x = x0; x <= x1; x++) {
        int alpha = (x - x0) * 65536 / (x1 - x0);
        int u = ((u1 - u0) * alpha + u0 * 65536) >> 16;
        int v = ((v1 - v0) * alpha + v0 * 65536) >> 16;

        if (u >= 0 && u < textureWidth && v >= 0 && v < textureHeight) {
            auto texel = texture[u + v * textureWidth];
            uint8_t r = (0xff & (texel>>16));
            uint8_t g = (0xff & (texel>>8));
            uint8_t b = (0xff & texel);
            r = (uint8_t) ((int16_t)(Fix16((int16_t)r) * lightInstensity));
            g = (uint8_t) ((int16_t)(Fix16((int16_t)g) * lightInstensity));
            b = (uint8_t) ((int16_t)(Fix16((int16_t)b) * lightInstensity));
            auto c = color(r,g,b);
            setPixel(x, y, c);
        }
    }
}

void drawTriangle(
    int16_t_Point2d v0, int16_t_Point2d v1, int16_t_Point2d v2,
    uint32_t *texture, int textureWidth, int textureHeight,
    Fix16 lightInstensity
) {
    if (v0.y > v1.y) swap(v0, v1);
    if (v0.y > v2.y) swap(v0, v2);
    if (v1.y > v2.y) swap(v1, v2);

    int totalHeight = v2.y - v0.y;

    // If triangle happens to be just a line, lets avoid it completely
    if (totalHeight == 0) return;

    // Drawing the upper part of the triangle
    for (int y = v0.y; y <= v1.y; y++) {
        int segmentHeight = v1.y - v0.y + 1;
        int alpha = ((y - v0.y) << 16) / totalHeight;
        int beta = ((y - v0.y) << 16) / segmentHeight;

        int x0 = v0.x + ((v2.x - v0.x) * alpha >> 16);
        int x1 = v0.x + ((v1.x - v0.x) * beta >> 16);

        int u0 = v0.u + ((v2.u - v0.u) * alpha >> 16);
        int u1 = v0.u + ((v1.u - v0.u) * beta >> 16);

        int v0_coord = v0.v + ((v2.v - v0.v) * alpha >> 16);
        int v1_coord = v0.v + ((v1.v - v0.v) * beta >> 16);

        drawHorizontalLine(x0, x1, y, u0, u1, v0_coord, v1_coord, texture, textureWidth, textureHeight, lightInstensity);
    }

    // Drawing the lower part of the triangle
    for (int y = v1.y + 1; y <= v2.y; y++) {
        int segmentHeight = v2.y - v1.y + 1;
        int alpha = ((y - v0.y) << 16) / totalHeight;
        int beta = ((y - v1.y) << 16) / segmentHeight;

        int x0 = v0.x + ((v2.x - v0.x) * alpha >> 16);
        int x1 = v1.x + ((v2.x - v1.x) * beta >> 16);

        int u0 = v0.u + ((v2.u - v0.u) * alpha >> 16);
        int u1 = v1.u + ((v2.u - v1.u) * beta >> 16);

        int v0_coord = v0.v + ((v2.v - v0.v) * alpha >> 16);
        int v1_coord = v1.v + ((v2.v - v1.v) * beta >> 16);

        drawHorizontalLine(x0, x1, y, u0, u1, v0_coord, v1_coord, texture, textureWidth, textureHeight, lightInstensity);
    }
}

void draw_center_square(int16_t cx, int16_t cy, int16_t sx, int16_t sy, color_t color)
{
    for(int16_t i=-sx/2; i<sx/2; i++)
    {
        for(int16_t j=-sy/2; j<sy/2; j++)
        {
            setPixel(cx+i, cy+j, color);
        }
    }
}

void draw_RotationVisualizer(fix16_vec2 camera_rot)
{
#ifdef LANDSCAPE_MODE
    // Points to rotate
    fix16_vec3 p_x     = { 0.0f,  ROTATION_VISUALIZER_LINE_WIDTH, 0.0f};
    fix16_vec3 p_y     = { ROTATION_VISUALIZER_LINE_WIDTH, 0.0f, 0.0f};
    fix16_vec3 p_z     = { 0.0f,  0.0f, ROTATION_VISUALIZER_LINE_WIDTH};
    // Rotations - x
    rotateOnPlane(p_x.y, p_x.z, camera_rot.x);
    rotateOnPlane(p_x.x, p_x.z, camera_rot.y);
    // Rotations - y
    rotateOnPlane(p_y.y, p_y.z, camera_rot.x);
    rotateOnPlane(p_y.x, p_y.z, camera_rot.y);
    // Rotations - z
    rotateOnPlane(p_z.y, p_z.z, camera_rot.x);
    rotateOnPlane(p_z.x, p_z.z, camera_rot.y);
#else
    // Points to rotate
    fix16_vec3 p_x     = {ROTATION_VISUALIZER_LINE_WIDTH,  0.0f,  0.0f};
    fix16_vec3 p_y     = { 0.0f, -ROTATION_VISUALIZER_LINE_WIDTH,  0.0f};
    fix16_vec3 p_z     = { 0.0f,  0.0f, ROTATION_VISUALIZER_LINE_WIDTH};
    // Rotations - x
    rotateOnPlane(p_x.x, p_x.z, camera_rot.x);
    rotateOnPlane(p_x.y, p_x.z, camera_rot.y);
    // Rotations - y
    rotateOnPlane(p_y.x, p_y.z, camera_rot.x);
    rotateOnPlane(p_y.y, p_y.z, camera_rot.y);
    // Rotations - z
    rotateOnPlane(p_z.x, p_z.z, camera_rot.x);
    rotateOnPlane(p_z.y, p_z.z, camera_rot.y);
#endif
    // Make sure there is no division with zero
    if (p_x.z == 0.0f) p_x.z = 0.001f;
    if (p_y.z == 0.0f) p_y.z = 0.001f;
    if (p_z.z == 0.0f) p_z.z = 0.001f;

#ifdef LANDSCAPE_MODE
    // Where to draw
    const auto offset_x = SCREEN_X - ROTATION_VISALIZER_EDGE_OFFSET - (int16_t) ROTATION_VISUALIZER_LINE_WIDTH;
    const auto offset_y = SCREEN_Y - ROTATION_VISALIZER_EDGE_OFFSET - (int16_t) ROTATION_VISUALIZER_LINE_WIDTH;
#else
    // Where to draw
    const auto offset_x = SCREEN_X - ROTATION_VISALIZER_EDGE_OFFSET - (int16_t) ROTATION_VISUALIZER_LINE_WIDTH;
    const auto offset_y =            ROTATION_VISALIZER_EDGE_OFFSET + (int16_t) ROTATION_VISUALIZER_LINE_WIDTH;
#endif
    // Draw actual lines
    line(((int16_t) p_x.x)+offset_x,((int16_t) p_x.y)+offset_y, offset_x, offset_y, color(255,0,0));
    line(((int16_t) p_y.x)+offset_x,((int16_t) p_y.y)+offset_y, offset_x, offset_y, color(0,255,0));
    line(((int16_t) p_z.x)+offset_x,((int16_t) p_z.y)+offset_y, offset_x, offset_y, color(0,0,255));
}

void bubble_sort(uint_fix16_t a[], int n) {
    for (int j = n; j > 1; --j)
        for (int i = 1; i < j; ++i)
            if (a[i - 1].fix16 < a[i].fix16)
                swap(a[i - 1], a[i]);
}
