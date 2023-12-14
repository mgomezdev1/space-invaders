#ifndef MTL_READER_HPP
#define MTL_READER_HPP

#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "../extensions/strUtils.hpp"
#include "../extensions/math.hpp"
#include "../shader.hpp"

using namespace std;
using namespace glm;

struct RawMtl {
    string mtlName;
    string fileName;

    vec3 ambient = vec3(1,1,1);
    vec3 diffuse = vec3(1,1,1);
    vec3 specular = vec3(1,1,1);
    float glossiness = 1; // AKA metallic, (inverse of) roughness, specular exponent/strength.
    float dissolve = 1; // AKA (inverse of) transparency
    vec3 emissive = vec3(0,0,0);

    string ambientMapFile = "";
    string diffuseMapFile = "";
    string specularMapFile = "";
    string glossinessMapFile = ""; // AKA metallic, (inverse of) roughness, specular exponent/strength.
    string dissolveMapFile = ""; // AKA (inverse of) transparency
    string emissiveMapFile = "";

    string normalMapFile = ""; // AKA bump map

    float refractiveIndex = 1; // AKA optical density
    int illumMode = 2;

    RawMtl(string name = "Unknown", string fileName = "unknown.mtl") {
        this->mtlName = name;
        this->fileName = fileName;
    }

    vector<string> GetFileNames() const {
        vector<string> result;
        result.push_back(ambientMapFile);
        result.push_back(diffuseMapFile);
        result.push_back(specularMapFile);
        result.push_back(glossinessMapFile);
        result.push_back(dissolveMapFile);
        result.push_back(emissiveMapFile);
        result.push_back(normalMapFile);
        return result;
    }
};

class MtlReader {
public:
    static vector<RawMtl> ReadMtl(const string& filename, bool verbose = false) {
        if (verbose) cout << "reading file " << filename << endl;
        ifstream infile(filename);
                
        vector<RawMtl> result;

        RawMtl* currentMaterial = nullptr;
        string line;

        while (NextNonComment(infile, line)) {
            stringstream ss(line);

            string directive;
            ss >> directive;
            // Normalize the directive so all characters are lowercase
            directive = ToLower(directive);
            if (directive == "newmtl") {
                if (currentMaterial != nullptr) {
                    result.push_back(*currentMaterial);
                }
                delete currentMaterial;
                line.erase(0, 7);
                currentMaterial = new RawMtl(line, filename);
                if (verbose) cout << "Parsing material " << line << endl;
            }
            else if (directive == "ka") {
                currentMaterial->ambient = ToVec3(ParseFloats(ss));
            }
            else if (directive == "kd") {
                currentMaterial->diffuse = ToVec3(ParseFloats(ss));
            }
            else if (directive == "ks") {
                currentMaterial->specular = ToVec3(ParseFloats(ss));
            }
            else if (directive == "ke") {
                currentMaterial->emissive = ToVec3(ParseFloats(ss));
            }
            else if (directive == "map_ka") {
                line.erase(0, directive.size() + 1);
                currentMaterial->ambientMapFile = GetPathRelativeToFile(filename, line);
                if (currentMaterial->diffuseMapFile == "") currentMaterial->diffuseMapFile = currentMaterial->ambientMapFile;
            }
            else if (directive == "map_kd") {
                line.erase(0, directive.size() + 1);
                currentMaterial->diffuseMapFile = GetPathRelativeToFile(filename, line);
                if (currentMaterial->ambientMapFile == "") currentMaterial->ambientMapFile = currentMaterial->diffuseMapFile;
            }
            else if (directive == "map_ks") {
                line.erase(0, directive.size() + 1);
                currentMaterial->specularMapFile = GetPathRelativeToFile(filename, line);
            }
            else if (directive == "map_ke") {
                line.erase(0, directive.size() + 1);
                currentMaterial->emissiveMapFile = GetPathRelativeToFile(filename, line);
            }
            else if (directive == "map_bump" || directive == "bump") {
                line.erase(0, directive.size() + 1);
                currentMaterial->normalMapFile = GetPathRelativeToFile(filename, line);
            }
            else if (directive == "ns") {
                float v; ss >> v;
                currentMaterial->glossiness = v;
            }
            else if (directive == "ni") {
                float v; ss >> v;
                currentMaterial->refractiveIndex = v;
            }
            else if (directive == "d") {
                float v; ss >> v;
                currentMaterial->dissolve = v;
            }
            else if (directive == "tr") {
                float v; ss >> v;
                currentMaterial->dissolve = 1 - v;
            }
            else if (directive == "map_ns") {
                line.erase(0, directive.size() + 1);
                currentMaterial->glossinessMapFile = GetPathRelativeToFile(filename, line);
            }
            else if (directive == "map_d") {
                line.erase(0, directive.size() + 1);
                currentMaterial->dissolveMapFile = GetPathRelativeToFile(filename, line);
            }
            else if (directive == "illum") {
                int v; ss >> v;
                currentMaterial->illumMode = v;
            }
        }
        infile.close();

        if (currentMaterial != nullptr)
            result.push_back(*currentMaterial);
            delete currentMaterial;

        //cout << "finished reading file " << filename << endl;
        return result;
    }
};

#endif