#ifndef MATH_HPP
#define MATH_HPP

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>

#include "exceptions.hpp"

using namespace glm;

static bool math_rand_initialized = false;
const float PI = 3.14159265358979323846264338327950288f;

vec3 SafeNormalize(const vec3& input) {
    if (input.x == 0 && input.y == 0 && input.z == 0) return input;
    return glm::normalize(input);
}

vec3 Step(const vec3& current, const vec3& target, float stepSize) {
    if (length(current - target) < stepSize)
        return target;
    return current + SafeNormalize(target - current) * stepSize;
}

vec3 CalculateTangent(const vec3& edge1, const vec2& deltaUv1, const vec3& edge2, const vec2& deltaUv2) {
    // The inverse of the determinant of the DeltaUV matrix
    float inverseNormalizationFactor = 1 / (deltaUv1.x * deltaUv2.y - deltaUv2.x * deltaUv1.y);
    vec3 result = inverseNormalizationFactor * (deltaUv2.y * edge1 - deltaUv1.y * edge2);
    //std::cout << "The calculated tangent is: " << VectorToStr(result) << endl;
    return result;
}
vec3 CalculateTangent(const vec3& p1, const vec2& uv1, const vec3& p2, const vec2& uv2, const vec3& p3, const vec2& uv3) {
    vec3 edge1 = p2 - p1;
    vec3 edge2 = p3 - p1;
    vec2 deltaUv1 = uv2 - uv1;
    vec2 deltaUv2 = uv3 - uv1;
    return CalculateTangent(edge1, deltaUv1, edge2, deltaUv2);
}

vec4 ToVec4(const std::vector<float>& data) {
    return vec4(data.at(0), data.at(1), data.at(2), data.at(3));
}
vec3 ToVec3(const std::vector<float>& data) {
    return vec3(data.at(0), data.at(1), data.at(2));
}
vec2 ToVec2(const std::vector<float>& data) {
    return vec2(data.at(0), data.at(1));
}

vec4 ToVec4(const vec3& xyz, float w) {
    return vec4(xyz.x, xyz.y, xyz.z, w);
}

inline float SquareMagnitude(const vec2& vector) {
    return vector.x * vector.x + vector.y * vector.y;
}
inline float SquareMagnitude(const vec3& vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}
inline float SquareMagnitude(const vec4& vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;
}

void InitializeRNG() {
    if (!math_rand_initialized) {
        std::srand(time(NULL));
        math_rand_initialized = true;
    }
}

// Generates a pseudo-random value in the range [min, max)
// 
// The first call of a Math randomizing function will initialize the randomizer automatically. 
int Random(int min, int max) {
    InitializeRNG();
    return min + (std::rand() % (max - min));
}

int RandomSign() {
    return Random(0, 2) ? -1 : 1;
}

float RandomValue(float min, float max) {
    InitializeRNG();
    if (!math_rand_initialized) {
        std::srand(time(NULL));
        math_rand_initialized = true;
    }
    float range = max - min;
    float intRange = RAND_MAX;
    return min + (std::rand() / intRange) * range;
}

vec2 RandomPointAround(vec2 point, float distance) {
    float angle = RandomValue(0, PI * 2);
    return vec2(distance * sin(angle), distance * cos(angle));
}
vec2 RandomPointAround(vec2 point, float minDistance, float maxDistance) {
    float distance = RandomValue(minDistance, maxDistance);
    return RandomPointAround(point, distance);
}
vec3 RandomPointAround(vec3 point, float distance) {
    float angle = RandomValue(0, PI * 2);
    float z = RandomValue(-1, 1);
    float angleMagnitude = sqrt(1 - z * z);
    return vec3(angleMagnitude * cos(angle), angleMagnitude * sin(angle), z) * distance;
}
vec3 RandomPointAround(vec3 point, float minDistance, float maxDistance) {
    float distance = RandomValue(minDistance, maxDistance);
    return RandomPointAround(point, distance);
}
vec4 RandomPointAround(vec4 point, float distance) {
    throw NotImplementedException("4D uniform random point");
}
vec4 RandomPointAround(vec4 point, float minDistance, float maxDistance) {
    float distance = RandomValue(minDistance, maxDistance);
    return RandomPointAround(point, distance);
}

#endif