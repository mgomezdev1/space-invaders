#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "vertex.hpp"

using namespace std;
using namespace glm;

// A simple data structure containing raw vertex data for a triangle
struct Tri {
    VertexData v1,v2,v3;
    Tri(VertexData v1, VertexData v2, VertexData v3) {
        this->v1=v1;
        this->v2=v2;
        this->v3=v3;
    }
    Tri() {}
};

// A more complex data structure whose behaviour may be altered
// Designed to be read-only.
class Triangle {
    Tri _getRawVertices() {
        return Tri(v1.GetData(), v2.GetData(), v3.GetData());
    }
    Tri cachedTri;
    bool cachedTriCalculated = false;
    Vertex v1;
    Vertex v2;
    Vertex v3;
public:
    Triangle(Vertex v1, Vertex v2, Vertex v3) {
        this->v1 = v1;
        this->v2 = v2;
        this->v3 = v3;
    }
    Tri GetRawVertices() {
        if (!cachedTriCalculated) {
            cachedTri = _getRawVertices();
            cachedTriCalculated = true;
        }
        return cachedTri;
    }

    void RecalculateTri() {
        cachedTriCalculated = false;
    }

    vec3 GetFaceNormal() {
        Tri tri = GetRawVertices();
        return normalize(cross(tri.v3.pos - tri.v2.pos, tri.v2.pos - tri.v1.pos));
    }

    vec3 GetCentroid() {
        Tri tri = GetRawVertices();
        return vec3(
            (tri.v1.pos.x, tri.v2.pos.x, tri.v3.pos.x) / 3,
            (tri.v1.pos.y, tri.v2.pos.y, tri.v3.pos.y) / 3,
            (tri.v1.pos.z, tri.v2.pos.z, tri.v3.pos.z) / 3
        );
    }
};


#endif