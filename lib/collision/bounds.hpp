#ifndef BOUNDS_HPP
#define BOUNDS_HPP

#include <glm/glm.hpp>

struct Bounds {
private:
    glm::vec3 minBound;
    glm::vec3 maxBound;
    glm::vec3 size;

public:
    Bounds(const glm::vec3& minBound = glm::vec3(0), const glm::vec3& maxBound = glm::vec3(0)) {
        this->minBound = minBound;
        this->maxBound = maxBound;
        size = maxBound - minBound;
    }
    bool Contains(float x, float y, float z) const {
        return Contains(glm::vec3(x,y,z));
    }
    bool Contains(const glm::vec3& point) const {
        return glm::all(glm::greaterThanEqual(point, minBound)) && glm::all(glm::lessThanEqual(point, maxBound));
    }
    inline glm::vec3 GetRelativePoint(float xRel, float yRel, float zRel) const {
        return GetRelativePoint(glm::vec3(xRel, yRel, zRel));
    }
    inline glm::vec3 GetRelativePoint(const glm::vec3& rel) const {
        return rel * size + minBound;
    }
    void SetMinBound(const glm::vec3& minBound) {
        this->minBound = minBound;
        size = maxBound - minBound;
    }
    glm::vec3 GetMinBound() const {
        return this->minBound;
    }
    void SetMaxBound(const glm::vec3& maxBound) {
        this->maxBound = maxBound;
        size = maxBound - minBound;
    }
    glm::vec3 GetMaxBound() const {
        return this->maxBound;
    }
    glm::vec3 SnapToBounds(const glm::vec3& position) const {
        glm::vec3 result = position;
        if (result.x < minBound.x) {
            result.x = minBound.x; 
        } else if (result.x > maxBound.x) {
            result.x = maxBound.x;
        }
        if (result.y < minBound.y) {
            result.y = minBound.y; 
        } else if (result.y > maxBound.y) {
            result.y = maxBound.y;
        }
        if (result.z < minBound.z) {
            result.z = minBound.z; 
        } else if (result.z > maxBound.z) {
            result.z = maxBound.z;
        }
        return result;
    }

    std::string ToString() {
        return "[" + VectorToStr(minBound) + ", " + VectorToStr(maxBound) + "]";
    }
};

#endif