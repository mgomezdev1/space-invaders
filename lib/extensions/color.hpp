#ifndef COLOR_HPP
#define COLOR_HPP

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

using namespace glm;

class Color {
    static float GetHue(float r, float g, float b) {
        // NOT IMPLEMENTED
        throw 1;
    }
    static float GetSaturation(float r, float g, float b) {
        // NOT IMPLEMENTED
        throw 1;
    }
    static float GetValue(float r, float g, float b) {
        return (r + g + b)/3;      
    }
    static vec3 Rgb2Hsl(float r, float g, float b) {
        return vec3(
            GetHue(r,g,b),
            GetSaturation(r,g,b),
            GetValue(r,g,b)
        );
    }
    static vec3 Rgb2Hsl(const vec3& rgb) {
        return Rgb2Hsl(rgb.r, rgb.b, rgb.b);
    }
    static vec4 Rgba2Hsla(float r, float g, float b, float a) {
        vec3 hsl = Rgb2Hsl(r,g,b);
        return vec4(hsl.x, hsl.y, hsl.z, a);
    }
    static vec4 Rgba2Hsla(const vec4& rgba) {
        return Rgba2Hsla(rgba.r, rgba.g, rgba.b, rgba.a);
    }
};

#endif