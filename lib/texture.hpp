#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL2/SDL.h>
#include <vector>

struct Pixel {
    uint8_t r,g,b;
    Pixel(uint8_t r, uint8_t g, uint8_t b) {
        this->r=r;
        this->g=g;
        this->b=b;
    }
    static std::string ToString(const Pixel& p) {
        return "(" + std::to_string((int)p.r) + " " + std::to_string((int)p.g) + " " + std::to_string((int)p.b) + ")";
    }
    std::string ToString() const {
        return Pixel::ToString(*this);
    }
};

struct Image {
    int width;
    int height;
    std::vector<Pixel> pixels;
    void Dump(std::vector<unsigned char>& buffer, bool invertY = true, bool invertX = false) const {
        if (!invertY && !invertX) {
            for (Pixel p : pixels) {
                buffer.push_back(p.r);
                buffer.push_back(p.g);
                buffer.push_back(p.b);
            }
        } else {
            for (int y = height - 1; y >= 0; --y) {
                int start = invertX ? ((y+1) * width - 1) : (( y ) * width);
                int limit = invertX ? (( y ) * width - 1) : ((y+1) * width);
                int step  = invertX ? -1 : 1;
                for (int idx = start; idx != limit; idx += step) {
                    buffer.push_back(pixels.at(idx).r);
                    buffer.push_back(pixels.at(idx).g);
                    buffer.push_back(pixels.at(idx).b);
                }
            }
        }
    }
    static Image Solid(const Pixel& color, int width = 1, int height = 1) {
        Image result;
        result.height = height;
        result.width = width;
        for (int i = 0; i < width * height; ++i) {
            result.pixels.push_back(color);
            std::cout << "Added color " << Pixel::ToString(color) << " to image" << std::endl; 
        }
        return result;
    }
    static std::string ToString(const Image& img) {
        std::string result = "";
        for (int i = 0; i < img.height; ++i) {
            for (int j = 0; j < img.width; ++j) {
                result += img.pixels[i * img.width + j].ToString() + " ";
            }
            result += "\n";
        }
        return result;
    }
    std::string ToString() const {
        return Image::ToString(*this);
    }
};

class Texture2D {
protected:
    GLuint handle;
    int width;
    int height;
public:
    Texture2D(GLuint handle = 0, int width = 0, int height = 0, GLenum warpMode = GL_REPEAT, GLenum minFilter = GL_LINEAR_MIPMAP_NEAREST, GLenum magFilter = GL_LINEAR) {
        this->handle = handle;
        this->width = width;
        this->height = height;
        this->warpMode = warpMode;
        this->minFilter = minFilter;
        this->magFilter = magFilter;
    }
    GLenum warpMode;
    GLenum minFilter;
    GLenum magFilter;
    GLuint GetHandle() const {return handle;}
    int GetWidth() const {return width;}
    int GetHeight() const {return height;}
};

#endif
