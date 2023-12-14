#ifndef OBJ_READER_HPP
#define OBJ_READER_HPP

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <memory>

#include "../geometry/vertex.hpp"
#include "../geometry/triangle.hpp"
#include "../geometry/mesh.hpp"
#include "../extensions/strUtils.hpp"
#include "../extensions/math.hpp"
#include "mtlReader.hpp"

using namespace std;
using namespace glm;

struct ObjData {
    string name;
    shared_ptr<RefMesh> mesh;
    RawMtl materialData;
    ObjData(string name = "Unknown", const RawMtl& materialData = RawMtl()) {
        //cout << "(Constructor) Creating Object Data for object " << name << endl;
        this->name = name;
        this->mesh = make_shared<RefMesh>();
        this->materialData = materialData;
    }
    ~ObjData() {
        //cout << "(Destructor) Deleting Object Data for object " << name << endl;

    }
};

IndexTuple ParseTuple(const string& text) {
    const char DELIM = '/';
    auto values = Split(text, DELIM);
    //cout << "Split tuple into " << VectorToStr(values) << endl;
    vector<int> parsedVals;
    for (string s : values) {
        if(s.length() == 0) {
            parsedVals.push_back(-1);
        } else {
            parsedVals.push_back(stoi(s) - 1);
        }
    }
    //cout << "Parsed values into " << VectorToStr(parsedVals) << endl;
    while (parsedVals.size() < 4) {
        parsedVals.push_back(-1);
    }

    IndexTuple result(parsedVals[0], parsedVals[1], parsedVals[2], parsedVals[3]);
    return result;
}

vector<IndexTuple> ParseFace(stringstream& lineStream) {
    auto values = SplitByWhitespace(lineStream);
    vector<IndexTuple> result;

    for (string s : values) {
        result.push_back(ParseTuple(s));
    }

    return result;
}

class ObjReader {
public:
    static vector<ObjData> ReadObj(const string& filename, bool verbose = false, bool calculateTangents = false) {
        if (verbose) cout << "reading file " << filename << endl;
        ifstream infile(filename);
                
        vector<ObjData> result;

        ObjData* currentObject = nullptr;
        RawMtl currentMatData;
        string line;

        // The offsets removed from each index
        // This is necessary to parse multiple objects,
        // Since each object goes to a different mesh,
        // But the .obj file holds global indices! (they don't reset back to 1)
        int vOffset = 0;
        int vtOffset = 0;
        int vnOffset = 0;
        while (NextNonComment(infile, line)) {
            stringstream ss(line);

            string directive;
            ss >> directive;

            if (directive == "o") {
                // Handle new object registration
                if (currentObject != nullptr) {
                    result.push_back(*currentObject);
                    vOffset  += currentObject->mesh->PositionCount();
                    vtOffset += currentObject->mesh->UvCount();
                    vnOffset += currentObject->mesh->NormalCount();
                }
                delete currentObject;
                line.erase(0, 2);
                currentObject = new ObjData(line, currentMatData);
                if (verbose) cout << "Parsing object " << line << endl;
            }
            else if (directive == "mtllib") {
                line.erase(0, 7);
                auto readMaterials = MtlReader::ReadMtl(GetPathRelativeToFile(filename, line), verbose);
                if (readMaterials.size() > 0)
                    currentMatData = readMaterials.at(0);
            }
            else if (directive == "v") {
                // Take the floats from the vertex position and register them
                vec3 vals = ToVec3(ParseFloats(ss));
                int idx = currentObject->mesh->RegisterPosition(vals);
                if (verbose) cout << "v " << idx << " -> " << VectorToStr(vals) << endl;
            }
            else if (directive == "vt") {
                // Take the floats from the UV position and register them
                vec2 vals = ToVec2(ParseFloats(ss));
                int idx = currentObject->mesh->RegisterUv(vals);
                if (verbose) cout << "vt" << idx << " -> " << VectorToStr(vals) << endl;
            }
            else if (directive == "vn") {
                // Take the floats from the vertex normals and register them
                vec3 vals = ToVec3(ParseFloats(ss));
                int idx = currentObject->mesh->RegisterNormal(vals);
                if (verbose) cout << "vn" << idx << " -> " << VectorToStr(vals) << endl;
            }
            else if (directive == "f") {
                auto verts = ParseFace(ss);
                if (verbose) cout << VectorToStr(verts, " ", IndexTuple::ToString) << " -> ";
                vector<int> handles;
                // Calculate the tangent if requested
                if (calculateTangents && verts.size() >= 3) {
                    vec3 tangent = CalculateTangent(
                        currentObject->mesh->GetPosition(verts.at(0).posIndex), currentObject->mesh->GetUv(verts.at(0).uvIndex),
                        currentObject->mesh->GetPosition(verts.at(1).posIndex), currentObject->mesh->GetUv(verts.at(1).uvIndex),
                        currentObject->mesh->GetPosition(verts.at(2).posIndex), currentObject->mesh->GetUv(verts.at(2).uvIndex)
                    );
                    int tangentIndex = currentObject->mesh->RegisterTangent(tangent);
                    for (int i = 0; i < verts.size(); ++i) {
                        verts[i].tangentIndex = tangentIndex;
                    }
                }
                // Register all face data and create a unique index for the combination (or retrieve the existing index)
                for (auto vert : verts) {
                    handles.push_back(currentObject->mesh->RegisterData(
                        vert.posIndex - vOffset, vert.uvIndex - vtOffset, vert.normalIndex - vnOffset, vert.colorIndex, vert.tangentIndex
                    ));
                }
                if (verbose) cout << VectorToStr(handles) << endl;
                // Manually triangulate the face if necessary
                for (int i = 0; i <= handles.size() - 3; ++i) {
                    // Add the triangle identified by three unique index handles
                    currentObject->mesh->AddTri(handles.at(0), handles.at(i+1), handles.at(i+2));
                }
            }
        }
        infile.close();

        if (currentObject != nullptr)
            result.push_back(*currentObject);
            delete currentObject;

        //cout << "finished reading file " << filename << endl;
        return result;
    }
};

#endif