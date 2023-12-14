#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "../extensions/strUtils.hpp"

using namespace std;
using namespace glm;

// Flags for the model loader to add certain attributes
typedef unsigned char MeshAttributeFlags;
static const MeshAttributeFlags MESH_UV_DATA = 1;               // 0001
static const MeshAttributeFlags MESH_NORMAL_DATA = 2;           // 0010
static const MeshAttributeFlags MESH_BASIC_DATA = 3;            // 0011
static const MeshAttributeFlags MESH_COLOR_DATA = 4;            // 0100
static const MeshAttributeFlags MESH_BASIC_AND_COLOR_DATA = 7;  // 0111
static const MeshAttributeFlags MESH_TANGENT_DATA = 8;          // 1000
static const MeshAttributeFlags MESH_NORMALMAPPABLE_DATA = 11;  // 1101
static const MeshAttributeFlags MESH_FULL_DATA = 15;            // 1111

struct VertexData {
    vec3 pos, normal;
    vec2 uv;
    vec4 color;
    vec3 tangent;
    VertexData(float x=0, float y=0, float z=0, float u=0, float v=0, float nx=0, float ny=1, float nz=0, float r=1, float g=1, float b=1, float a=1, float tx = 1, float ty = 0, float tz = 0) {
        pos.x=x;
        pos.y=y;
        pos.z=z;
        normal.x=nx;
        normal.y=ny;
        normal.z=nz;
        uv.x = u;
        uv.y = v;
        color.r=r;
        color.g=g;
        color.b=b;
        color.a=a;
        tangent.x=tx;
        tangent.y=ty;
        tangent.z=tz;
    }
    static VertexData FromVectors(vec3 position=vec3(0,0,0), vec2 uv=vec2(0,0), vec3 normal=vec3(0,1,0), vec4 color=vec4(1,1,1,1), vec3 tangent=vec3(1,0,0)) {
        return VertexData(
            position.x, position.y, position.z,
            uv.x, uv.y,
            normal.x, normal.y, normal.z,
            color.r, color.g, color.b, color.a,
            tangent.x, tangent.y, tangent.z
        );
    }
    string ToString() const {
        return "[" + VectorToStr(pos) + ", " + VectorToStr(uv) + ", " + VectorToStr(normal) + ", " + VectorToStr(color) + ", " + VectorToStr(tangent) + "]";
    }
    static string ToString(VertexData data) {
        return data.ToString();
    }

    // DATA LAYOUT
    // x y z u v nx ny nz r g b a
    // 12 floats, 48 bytes
    // position offset: 0B, 
    // uv offset:       12B, 
    // normal offset:   20B, 
    // color offset:    32B
    void DumpData(vector<float>& container, MeshAttributeFlags attributes = MESH_BASIC_AND_COLOR_DATA) const {
        container.push_back(pos.x);
        container.push_back(pos.y);
        container.push_back(pos.z);
        if (attributes & MESH_UV_DATA) {
            container.push_back(uv.x);
            container.push_back(uv.y);
        }
        if (attributes & MESH_NORMAL_DATA) {
            container.push_back(normal.x);
            container.push_back(normal.y);
            container.push_back(normal.z);
        }
        if (attributes & MESH_COLOR_DATA) {
            container.push_back(color.r);
            container.push_back(color.g);
            container.push_back(color.b);
            container.push_back(color.a);
        }
        if (attributes & MESH_TANGENT_DATA) {
            container.push_back(tangent.x);
            container.push_back(tangent.y);
            container.push_back(tangent.z);
        }
    }
};

struct IndexTuple {
    int posIndex, uvIndex, normalIndex, colorIndex, tangentIndex;
    IndexTuple(int posIndex = -1, int uvIndex = -1, int normalIndex = -1, int colorIndex = -1, int tangentIndex = -1) {
        this->posIndex=posIndex;
        this->uvIndex=uvIndex;
        this->normalIndex=normalIndex;
        this->colorIndex=colorIndex;
        this->tangentIndex=tangentIndex;
    }
    string ToString() const {
        return "[" + std::to_string(posIndex) + ", " + std::to_string(uvIndex) + ", " + std::to_string(normalIndex) + ", " + std::to_string(colorIndex) + ", " + std::to_string(tangentIndex) + "]";
    }
    static string ToString(IndexTuple obj) {
        return obj.ToString();
    }
};

// Source of the magic number and formula: https://stackoverflow.com/questions/7110301/generic-hash-for-tuples-in-unordered-map-unordered-set
struct TupleHash {
    size_t operator()(const IndexTuple& key) const {
        size_t seed;
        seed  = key.posIndex + 0x9e3779b9;
        seed ^= key.uvIndex + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= key.normalIndex + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= key.colorIndex + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= key.tangentIndex + 0x9e3779b9 + (seed<<6) + (seed>>2);
        return seed;
    }
};

struct TupleEqual {
    size_t operator()(const IndexTuple& a, const IndexTuple& b) const {
        return
            a.posIndex == b.posIndex &&
            a.uvIndex == b.uvIndex &&
            a.normalIndex == b.normalIndex &&
            a.colorIndex == b.colorIndex &&
            a.tangentIndex == b.tangentIndex;
    }
};

class VertexDataProvider {
protected:
    vector<vec3> positionProvider;
    vector<vec2> uvProvider;
    vector<vec3> normalProvider;
    vector<vec4> colorProvider;
    vector<vec3> tangentProvider;
    
    vector<VertexData> fullDataProvider;
    unordered_map<IndexTuple, int, TupleHash, TupleEqual> indexCompressor;
public:
    vec3 defaultPos;
    vec2 defaultUv;
    vec3 defaultNormal = vec3(0,1,0);
    vec4 defaultColor = vec4(1,1,1,1);
    vec3 defaultTangent = vec3(1,0,0);

    int RegisterPosition(const vec3& pos) {
        int index = positionProvider.size();
        positionProvider.push_back(pos);
        return index;
    }
    int RegisterUv(const vec2& uv) {
        int index = uvProvider.size();
        uvProvider.push_back(uv);
        return index;
    }
    int RegisterNormal(const vec3& normal) {
        int index = normalProvider.size();
        normalProvider.push_back(normal);
        return index;
    }
    int RegisterColor(const vec4& color) {
        int index = colorProvider.size();
        colorProvider.push_back(color);
        return index;
    }
    int RegisterTangent(const vec3& tangent) {
        int index = tangentProvider.size();
        tangentProvider.push_back(tangent);
        return index;
    }

    vec3 GetPosition(int index) const {
        if (index < 0 || index >= positionProvider.size()) return defaultPos;
        return positionProvider[index];
    }
    vec2 GetUv(int index) const {
        if (index < 0 || index >= uvProvider.size()) return defaultUv;
        return uvProvider[index];
    }
    vec3 GetNormal(int index) const {
        if (index < 0 || index >= normalProvider.size()) return defaultNormal;
        return normalProvider[index];
    }
    vec4 GetColor(int index) const {
        if (index < 0 || index >= colorProvider.size()) return defaultColor;
        return colorProvider[index];
    }
    vec3 GetTangent(int index) const {
        if (index < 0 || index >= tangentProvider.size()) return defaultTangent;
        return tangentProvider[index];
    }

    int PositionCount() const {return positionProvider.size();}
    int UvCount() const {return uvProvider.size();}
    int NormalCount() const {return normalProvider.size();}
    int ColorCount() const {return colorProvider.size();}
    int TangentCount() const {return tangentProvider.size();}

    VertexData BuildData(const IndexTuple& indices) const {
        return VertexData::FromVectors(
            GetPosition(indices.posIndex),
            GetUv(indices.uvIndex),
            GetNormal(indices.normalIndex),
            GetColor(indices.colorIndex),
            GetTangent(indices.tangentIndex)
        );
    }

    int RegisterData(const IndexTuple& indices) {
        // Check if the element exists
        if (indexCompressor.find(indices) == indexCompressor.end()) {
            // If it doesn't, register it
            int newIndex = fullDataProvider.size();
            fullDataProvider.push_back(BuildData(indices));
            indexCompressor[indices] = newIndex;
            return newIndex;
        }
        return indexCompressor[indices];
    }
    int RegisterData(int posIndex = -1, int uvIndex = -1, int normalIndex = -1, int colorIndex = -1, int tangentIndex = -1) {
        auto tuple = IndexTuple(posIndex, uvIndex, normalIndex, colorIndex, tangentIndex);
        return RegisterData(tuple);
    }

    vector<VertexData> GetData() const {
        return fullDataProvider;
    }

    // DATA LAYOUT
    // x y z u v nx ny nz r g b a
    // 12 floats, 48 bytes
    // position offset: 0B, 
    // uv offset:       12B, 
    // normal offset:   20B, 
    // color offset:    32B
    vector<float> DumpData(MeshAttributeFlags attributes = MESH_BASIC_AND_COLOR_DATA) const {
        vector<float> result;
        for (VertexData d : GetData()) {
            d.DumpData(result, attributes);
        }
        return result;
    }
};

class Vertex {
protected:
    VertexData _getData() const {
        return cachedData;
    }
    VertexData cachedData;
    bool cachedDataCalculated = false;
public:
    VertexData GetData() {
        if (!cachedDataCalculated) {
            cachedData = _getData();
            cachedDataCalculated = true;
        }
        return cachedData;
    }
    void RecalculatePos() {
        cachedDataCalculated = false;
    }
    Vertex(const VertexData& data) {
        this->cachedData = data;
        cachedDataCalculated = true;
    }
    Vertex(float x=0, float y=0, float z=0, float u=0, float v=0, float nx=0, float ny=1, float nz=0, float r=0, float g=0, float b=0, float a=1) {
        this->cachedData = VertexData(x,y,z,u,v,nx,ny,nz,r,g,b,a);
        cachedDataCalculated = true;
    }
};

class RefVertex : public Vertex {
protected:
    VertexDataProvider* dataProvider;
    IndexTuple refIndices;
    VertexData _getData() const {
        return dataProvider->BuildData(refIndices);
    }
public:
    RefVertex(const IndexTuple& refIndices) {
        this->refIndices = refIndices;
    }
    RefVertex(int posIndex = -1, int uvIndex = -1, int normalIndex = -1, int colorIndex = -1) {
        this->refIndices = IndexTuple(posIndex, uvIndex, normalIndex, colorIndex);
    }
};

#endif