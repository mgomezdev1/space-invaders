#ifndef SHADER_HPP
#define SHADER_HPP

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <unordered_set>

#include "texture.hpp"
#include "extensions/collectionUtils.hpp"
#include "extensions/strUtils.hpp"

using namespace std;
using namespace glm;

const string UNIFORM_LIGHT_COUNT = "u_LightCount";
const string UNIFORM_LIGHT_DATA = "u_Lights";

/**
* LoadShaderAsString takes a filepath as an argument and will read line by line a file and return a string that is meant to be compiled at runtime for a vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*       LoadShaderAsString("./shaders/filepath");
*
* @param filename Path to the shader file
* @return Entire file stored as a single string 
*/
std::string LoadShaderAsString(const std::string& filename){
    // Resulting shader program loaded as a single string
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if(myFile.is_open()){
        while(std::getline(myFile, line)){
            result += line + '\n';
        }
        myFile.close();
    }

    return result;
}

/**
* CompileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*	    Compile a vertex shader: 	CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
*       Compile a fragment shader: 	CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
*
* @param type We use the 'type' field to determine which shader we are going to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
	// Compile our shaders
	GLuint shaderObject;

	// Based on the type passed in, we create a shader object specifically for that
	// type.
	if(type == GL_VERTEX_SHADER){
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}else if(type == GL_FRAGMENT_SHADER){
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	// The source of our shader
	glShaderSource(shaderObject, 1, &src, nullptr);
	// Now compile our shader
	glCompileShader(shaderObject);

	// Retrieve the result of our compilation
	int result;
	// Our goal with glGetShaderiv is to retrieve the compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE){
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length]; // Could also use alloca here.
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if(type == GL_VERTEX_SHADER){
			std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
		}else if(type == GL_FRAGMENT_SHADER){
			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
		}
		// Reclaim our memory
		delete[] errorMessages;

		// Delete our broken shader
		glDeleteShader(shaderObject);

		return 0;
	}

  return shaderObject;
}

/**
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
*
* @param vertexShaderSource Vertex source code as a string
* @param fragmentShaderSource Fragment shader source code as a string
* @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){

    // Create a new program object
    GLuint programObject = glCreateProgram();

    // Compile our shaders
    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link our two shader programs together.
	// Consider this the equivalent of taking two .cpp files, and linking them into
	// one executable file.
    glAttachShader(programObject,myVertexShader);
    glAttachShader(programObject,myFragmentShader);
    glLinkProgram(programObject);

    // Validate our program
    glValidateProgram(programObject);

    // Once our final program Object has been created, we can
	// detach and then delete our individual shaders.
    glDetachShader(programObject,myVertexShader);
    glDetachShader(programObject,myFragmentShader);
	// Delete the individual shaders once we are done
    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);

    return programObject;
}

/**
* Create the graphics pipeline
*
* @return void
*/
GLuint CreateGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath){

    std::string vertexShaderSource   = LoadShaderAsString(vertexShaderPath);
    std::string fragmentShaderSource = LoadShaderAsString(fragmentShaderPath);

	return CreateShaderProgram(vertexShaderSource,fragmentShaderSource);
}

struct LightData {
	vec3 position;
	GLint active;
    vec3 color;
	float _DUMMY_PADDING0;
    vec4 attenuation;
	float ambientPower;
	float diffusePower;
	float specularPower;
	float _DUMMY_PADDING1;
	LightData(vec3 position = {0,0,0}, vec3 color = {1,1,1}, vec4 attenuation = {1, 0.5f, 0.5f, 0}, float ambientPower = 1, float diffusePower = 1, float specularPower = 1) {
        this->position = position;
		this->color = color;
        this->attenuation = attenuation;   
		this->ambientPower = ambientPower;
		this->diffusePower = diffusePower;
		this->specularPower = specularPower; 
		this->active = 1;
    }
};

class Shader {
private:
	GLuint handle;
	bool supportsLights = false;
	unordered_set<string> errorDisplayed;
public:
	Shader(GLuint shaderHandle = 0) {
		handle = shaderHandle;
		this->supportsLights = shaderHandle != 0 && glGetUniformLocation(handle, UNIFORM_LIGHT_COUNT.c_str()) >= 0;
	}
	GLuint GetHandle() const {return handle;}

	void Use() {
		glUseProgram(handle);
	}
	void Destroy() {
		glDeleteProgram(handle);
	}

	GLint SetUniformMatrix(const string& uniformName, const mat4& matrix, bool warn = false) {
        GLint uniformLocation = glGetUniformLocation(handle, uniformName.c_str());
        if(uniformLocation >= 0){
            glUniformMatrix4fv(uniformLocation,1,GL_FALSE,&matrix[0][0]);
        } else if (warn) {
            WarnShaderUniform(uniformName);
        }
        return uniformLocation;
    }
    GLint SetUniformVector(const string& uniformName, const vec4& vector, bool warn = false) {
        GLint uniformLocation = glGetUniformLocation(handle, uniformName.c_str());
        if(uniformLocation >= 0){
            glUniform4fv(uniformLocation, 1, &vector[0]);
        } else if (warn) {
            WarnShaderUniform(uniformName);
        }
        return uniformLocation;
    }
	GLint SetUniformVector3(const string& uniformName, const vec3& vector, bool warn = false) {
        GLint uniformLocation = glGetUniformLocation(handle, uniformName.c_str());
        if(uniformLocation >= 0){
            glUniform3fv(uniformLocation, 1, &vector[0]);
        } else if (warn) {
            WarnShaderUniform(uniformName);
        }
        return uniformLocation;
    }
    GLint SetUniformValue(const string& uniformName, float value, bool warn = false) {
        GLint uniformLocation = glGetUniformLocation(handle, uniformName.c_str());
        if(uniformLocation >= 0){
            glUniform1f(uniformLocation, value);
        } else if (warn) {
            WarnShaderUniform(uniformName);
        }
        return uniformLocation;
    }
    GLint SetUniformInt(const string& uniformName, int value, bool warn = false) {
        GLint uniformLocation = glGetUniformLocation(handle, uniformName.c_str());
        if(uniformLocation >= 0){
            glUniform1i(uniformLocation, value);
        } else if (warn) {
            WarnShaderUniform(uniformName);
        }
        return uniformLocation;
    }
	GLint SetUniformBlock(const string& blockName, GLuint buffer, GLsizeiptr size) {
		GLint uniformIndex = glGetUniformBlockIndex(handle, blockName.c_str());
		if (uniformIndex >= 0) {
			glBindBuffer(GL_UNIFORM_BUFFER, buffer);
			glUniformBlockBinding(handle, uniformIndex, buffer);
			glBindBufferRange(GL_UNIFORM_BUFFER, uniformIndex, buffer, 0, size);
		} else {
			WarnShaderUniform(blockName);
		}
	}
    vector<GLint> SetUniformTextures(size_t count, const string* textureNames, const Texture2D* textures, bool warn = false) {
        vector<GLint> locations;
        for (int i = 0; i < count; i++) {
            string uniformName = textureNames[i];
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i].GetHandle());
            GLint uniformLocation = glGetUniformLocation(handle, uniformName.c_str());
            if(uniformLocation >= 0){
                glUniform1i(uniformLocation,i);
            } else if (warn) {
                WarnShaderUniform(uniformName);
            }
            locations.push_back(uniformLocation);
        }
        return locations;
    }
    void SetLightUniforms(GLuint ubo, size_t max_lights) {
		//std::cout << "Setting light uniforms for shader " << handle << ", with supportsLight = " << supportsLights << endl;
		if (!supportsLights) return;
        SetUniformBlock("LightBlock", ubo, max_lights + sizeof(GLuint));
    }
	void SetLightUniformsRaw(vector<LightData*> lights) {
		if (!supportsLights) return;
		SetUniformInt("lightCount", lights.size(), true);
		for (int i = 0; i < lights.size(); ++i) {
			SetUniformVector3("lights[" + to_string(i) + "].position", lights[i]->position, true);
			SetUniformVector3("lights[" + to_string(i) + "].color", lights[i]->color, true);
			SetUniformVector ("lights[" + to_string(i) + "].attenuation", lights[i]->attenuation, true);
			SetUniformValue  ("lights[" + to_string(i) + "].ambientPower", lights[i]->ambientPower, true);
			SetUniformValue  ("lights[" + to_string(i) + "].diffusePower", lights[i]->diffusePower, true);
			SetUniformValue  ("lights[" + to_string(i) + "].specularPower", lights[i]->specularPower, true);
			SetUniformInt    ("lights[" + to_string(i) + "].on", lights[i]->active, true);
		}
	}

    void WarnShaderUniform(const string& uniformName) {
        if (errorDisplayed.find(uniformName) == errorDisplayed.end()) {
            errorDisplayed.insert(uniformName);
            std::cout << "uniform " << uniformName << " not present in shader " << handle << ". Perhaps it's mispelled or it was optimized away.\n";
        }
    }

	Shader* EnableLighting() {
		supportsLights = true;
		return this;
	}
};

class Material {
protected:
	unordered_map<string, float> valueProperties;
	unordered_map<string, vec4> vecProperties;
	unordered_map<string, Texture2D> textureProperties;
	Material(){}
public:
	Shader* shader;
	Material(Shader* shader) {
		this->shader = shader;
	}

	void SetValue(const string& name, float value) {
		valueProperties[name] = value;
	}	
	void SetVector(const string& name, const vec4& vector) {
		vecProperties[name] = vector;
	}
	void SetTexture(const string& name, const Texture2D& texture) {
		textureProperties[name] = texture;
	}
	
	float GetValue(const string& name) const {
		return valueProperties.at(name);
	}
	vec4 GetVector(const string& name) const {
		return vecProperties.at(name);
	}
	Texture2D GetTexture(const string& name) const {
		return textureProperties.at(name);
	}

	vector<string> GetValueProperties() const {
		return GetKeys<string, float>(valueProperties);
	}
	vector<string> GetVectorProperties() const {
		return GetKeys<string, vec4>(vecProperties);
	}
	vector<string> GetTextureProperties() const {
		return GetKeys<string, Texture2D>(textureProperties);
	}

    void SetMaterialProperties(bool warn = false) {
        for (string& s : GetValueProperties()) {
            shader->SetUniformValue(s, GetValue(s), warn);
        }
        for (string& s : GetVectorProperties()) {
            shader->SetUniformVector(s, GetVector(s), warn);
        }
        vector<string> texNames = GetTextureProperties();
        vector<Texture2D> textures;
        for (string& s : texNames) {
            textures.push_back(GetTexture(s));
        }
        shader->SetUniformTextures(texNames.size(), texNames.data(), textures.data(), warn);
    }
};

class UnlitMaterial : public Material {
protected:
	UnlitMaterial(){}
public:
	UnlitMaterial(Shader* shader, vec4 color, Texture2D texture) {
		this->shader = shader;
		SetColor(color);
		SetMainTexture(texture);
	}
	vec4 GetColor() const {return GetVector("u_Color");}
	Texture2D GetMainTexture() const {return GetTexture("u_Texture");}

	void SetColor(vec4 color) {SetVector("u_Color", color);}
	void SetMainTexture(Texture2D texture) {SetTexture("u_Texture", texture);}	
};

class LitMaterial : public UnlitMaterial {
protected:
	LitMaterial(){}
public:
	LitMaterial(Shader* shader, vec4 color, Texture2D texture, 
		vec4 ambient, vec4 diffuse, vec4 specular, float glossiness, Texture2D normalMap, Texture2D glossinessMap
	) {
		this->shader = shader;
		SetColor(color);
		SetMainTexture(texture);
		SetAmbient(ambient);
		SetDiffuse(diffuse);
		SetSpecular(specular);
		SetGlossiness(glossiness);
		SetNormalMap(normalMap);
		SetGlossinessMap(glossinessMap);
	}
	Texture2D GetNormalMap() const {return GetTexture("u_NormalMap");}
	Texture2D GetGlossinessMap() const {return GetTexture("u_GlossinessMap");}
	vec4 GetAmbient() const {return GetVector("u_Ambient");}
	vec4 GetDiffuse() const {return GetVector("u_Diffuse");}
	vec4 GetSpecular() const {return GetVector("u_Specular");}
	float GetGlossiness() const {return GetValue("u_Glossiness");}
	
	void SetNormalMap(Texture2D texture) {SetTexture("u_NormalMap", texture);}
	void SetGlossinessMap(Texture2D texture) {SetTexture("u_GlossinessMap", texture);}
	void SetAmbient(vec4 color) {SetVector("u_Ambient", color);}
	void SetDiffuse(vec4 color) {SetVector("u_Diffuse", color);}
	void SetSpecular(vec4 color) {SetVector("u_Specular", color);}
	void SetGlossiness(float value) {SetValue("u_Glossiness", value);}
};

// Full material is meant to capture the full specification available in .mtl files
// It is not fully implemented, but it holds everything that the MTL parser is capable of parsing.
// For MTL files, the "MainTexture" field is considered to refer to the ambient texture.
// "IllumMode" is converted a float internally, and should realistically be used externally to set the appropriate shader program.
// As for nomeclature, "Texture" is used when the image contains colors, "Map" is used when it contains numerical information such as floats or vectors.
class FullMaterial : public LitMaterial {
protected:
	FullMaterial(){}
public:
	FullMaterial(Shader* shader, vec4 color, Texture2D normalMap,
		vec4 ambient, Texture2D ambientTexture, 
		vec4 diffuse, Texture2D diffuseTexture,
		vec4 specular, Texture2D specularTexture, 
		float glossiness, Texture2D glossinessMap, 
		vec4 emissive, Texture2D emissiveTexture, 
		float dissolve, Texture2D dissolveMap, 
		float refractiveIndex, int illumMode
	) {
		this->shader = shader;
		SetColor(color);
		SetAmbient(ambient);
		SetDiffuse(diffuse);
		SetSpecular(specular);
		SetGlossiness(glossiness);
		SetEmissive(emissive);
		SetDissolve(dissolve);
		SetRefractiveIndex(refractiveIndex);
		SetIllumMode(illumMode);
		// Textures
		SetNormalMap(normalMap);
		SetGlossinessMap(glossinessMap);
		SetAmbientTexture(ambientTexture);
		SetDiffuseTexture(diffuseTexture);
		SetSpecularTexture(specularTexture);
		SetEmissiveTexture(emissiveTexture);
		SetDissolveMap(dissolveMap);
	}
	// Overrides and linked methods
	Texture2D GetMainTexture() const {return GetTexture("u_AmbientTexture");}
	Texture2D GetAmbientTexture() const {return GetMainTexture();}
	void SetMainTexture(Texture2D texture) {SetTexture("u_AmbientTexture", texture);}
	void SetAmbientTexture(Texture2D texture) {SetMainTexture(texture);}

	// New supported values	
	vec4 GetDiffuse() const {return GetVector("u_Diffuse");}
	vec4 GetSpecular() const {return GetVector("u_Specular");}
	vec4 GetEmissive() const {return GetVector("u_Emissive");}
	float GetDissolve() const {return GetValue("u_Dissolve");}
	float GetRefractiveIndex() const {return GetValue("u_RefractiveIndex");}
	Texture2D GetDiffuseTexture() const {return GetTexture("u_DiffuseTexture");}
	Texture2D GetSpecularTexture() const {return GetTexture("u_SpecularTexture");}
	Texture2D GetEmissiveTexture() const {return GetTexture("u_EmissiveTexture");}
	Texture2D GetDissolveMap() const {return GetTexture("u_DissolveMap");}
	int GetIllumMode() const {return static_cast<int>(GetValue("u_IllumMode"));}

	void SetDiffuse(vec4 color) {SetVector("u_Diffuse", color);}
	void SetSpecular(vec4 color) {SetVector("u_Specular", color);}
	void SetEmissive(vec4 color) {SetVector("u_Emissive", color);}
	void SetDissolve(float value) {SetValue("u_Dissolve", value);}
	void SetRefractiveIndex(float value) {SetValue("u_RefractiveIndex", value);}
	void SetDiffuseTexture(Texture2D texture) {SetTexture("u_DiffuseTexture", texture);}
	void SetSpecularTexture(Texture2D texture) {SetTexture("u_SpecularTexture", texture);}
	void SetEmissiveTexture(Texture2D texture) {SetTexture("u_EmissiveTexture", texture);}
	void SetDissolveMap(Texture2D texture) {SetTexture("u_DissolveMap", texture);}
	void SetIllumMode(int value) {SetValue("u_IllumMode", static_cast<float>(value));}
};

#endif