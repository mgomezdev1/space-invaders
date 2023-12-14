#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "vertex.hpp"
#include "triangle.hpp"

using namespace std;
using namespace glm;

struct RefTri {
    int i1, i2, i3;
    RefTri(int i1, int i2, int i3) {
        this->i1 = i1;
        this->i2 = i2;
        this->i3 = i3;
    }
};

// Mesh Interface
class IMesh {
public:
    virtual vector<GLfloat> GetArrayBuffer(MeshAttributeFlags attributes = MESH_BASIC_AND_COLOR_DATA) const = 0;
    virtual vector<GLuint> GetElementArrayBuffer() const = 0;
};

// A RefMesh is made to work with element buffers
// It stores each vertex/uv/normal/color combination only once
// And provides methods for registering these data points
class RefMesh : public IMesh, public VertexDataProvider {
protected:
    vector<RefTri> triangles;
public:
    void AddTri(RefTri vertexIndices) {
        triangles.push_back(vertexIndices);
    }
    void AddTri(int v1, int v2, int v3) {
        AddTri(RefTri(v1, v2, v3));
    }
    vector<GLuint> GetElementArrayBuffer() const {
        vector<GLuint> result;
        for (RefTri rt : triangles) {
            result.push_back(rt.i1);
            result.push_back(rt.i2);
            result.push_back(rt.i3);
        }
        return result;
    }
    vector<GLfloat> GetArrayBuffer(MeshAttributeFlags attributes = MESH_BASIC_AND_COLOR_DATA) const {
        return DumpData(attributes);
    }
};

// A RawMesh is designed to work with a basic Array Buffer
// It allows the use of element buffers objects anyway
// It stores each vertex of the mesh per-tri.
class RawMesh : public IMesh {
protected:
    vector<Triangle> triangles;
public:
    vector<GLuint> GetElementArrayBuffer() const {
        vector<GLuint> result;
        for (int i = 0; i < triangles.size() * 3; ++i) {
            result.push_back(i);
        }
        return result;
    }
    vector<GLfloat> GetArrayBuffer(MeshAttributeFlags attributes = MESH_BASIC_AND_COLOR_DATA) const {
        vector<GLfloat> result;
        for (Triangle tri : triangles) {
            auto vertices = tri.GetRawVertices();
            vertices.v1.DumpData(result, attributes);
            vertices.v2.DumpData(result, attributes);
            vertices.v3.DumpData(result, attributes);
        }
    }
};

// The Meshhandle represents a set of handles for a mesh that OpenGL is able to reinterpret as buffers.
// It also includes necessary information about the number of elements contained for drawing.
struct MeshHandle {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int elementCount;
    MeshHandle(GLuint vao = 0, GLuint vbo = 0, GLuint ebo = 0, int elementCount = 0) {
        this->vao = vao;
        this->vbo = vbo;
        this->ebo = ebo;
        this->elementCount = elementCount;
    }
};

#endif