#ifndef BULK_RENDERING_HPP
#define BULK_RENDERING_HPP

#include <vector>

#include "mesh.hpp"
#include "../gameObject.hpp"

using namespace glm;
 
struct Instance {
    GameObject* object;
    std::vector<vec4> extraAttribs;
    Instance(GameObject* object) {
        this->object = object;
    }
    Instance* AddAttribute(vec4 attrib) {
        extraAttribs.push_back(attrib);
        return this;
    }
    void Dump(std::vector<vec4>& target, int extraAttribCount) {
        if (extraAttribCount != extraAttribs.size()) {
            std::cerr << "Instance of " << object->objectName << " with wrong attribute count detected during bulk rendering! Expected " << extraAttribCount << " but instance has " << extraAttribs.size() << " extra attributes." << std::endl;
        }

        mat4 model = object->GetGlobalTransform().GetModelMatrix();
        for (int i = 0; i < 4; ++i)
            target.push_back(model[i]);
        for (int i = 0; i < extraAttribCount; ++i)
            target.push_back(extraAttribs.at(i));
    }
};

class InstancedRenderer {
private:
    int extraVectors = 0;
public:
    GLuint buffer;
    MeshHandle* globalMesh;
    Material* globalMaterial;
    std::vector<Instance*> instances;

    InstancedRenderer() {}
    InstancedRenderer(MeshHandle* mesh, Material* material, GLuint buffer, int extraAttribCount = 0) {
        globalMesh = mesh;
        globalMaterial = material;
        extraVectors = extraAttribCount;
        this->buffer = buffer;
    }
    ~InstancedRenderer() {
        glDeleteBuffers(1, &buffer);
    }

    static InstancedRenderer WithNewBuffer(MeshHandle* mesh, Material* material, int extraAttribCount = 0) {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        auto result = InstancedRenderer(mesh, material, buffer, extraAttribCount);
        result.Initialize();
        return result;
    }
    InstancedRenderer* Initialize() {
        // Temporary while fixing
        return this;

        // instance attributes
        std::size_t vec4Size = sizeof(glm::vec4);
        glBindVertexArray(globalMesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        const int INSTANCE_LAYOUT_START = 5;
        for (int i = 0; i < 4 + extraVectors; i++) {
            glEnableVertexAttribArray(i + INSTANCE_LAYOUT_START); 
            glVertexAttribPointer(i + INSTANCE_LAYOUT_START, 4, GL_FLOAT, GL_FALSE, (4 + extraVectors) * vec4Size, (void*)(i * vec4Size));
            glVertexAttribDivisor(i + INSTANCE_LAYOUT_START, 1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        return this;
    }
    InstancedRenderer* AddInstance(Instance* instance) {
        instances.push_back(instance);
        instance->object->isInstanced = true;
        return this;
    }
    InstancedRenderer* AddInstances(std::vector<Instance*> instances) {
        for (Instance* instance: instances)
        {
            AddInstance(instance);
        }
        return this;
    }
    bool AnyEnabled() {
        for (Instance* instance: instances) {
            if (instance->object->IsEnabled()) return true;
        }
        return false;
    }
    void Draw(const mat4& vMatrix, const mat4& pMatrix, const mat4& vpMatrix, const vector<LightData*>& lights, float time = 0, bool warnMissingShaderUniforms = false, bool verbose = false) {
        std::vector<vec4> models;
        int enabledInstances = 0;
        for (int i = 0; i < instances.size(); i++) {
            if (!instances.at(i)->object->IsEnabled()) continue;

            instances.at(i)->Dump(models, extraVectors);
            enabledInstances++;
        }
        
        if (verbose) std::cout << "Bulk-drawing " << enabledInstances << "/" << instances.size() << " instances." << std::endl;
        if (enabledInstances == 0) return;
        
        Shader* globalShader = globalMaterial->shader;
        globalShader->Use();
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(vec4), models.data(), GL_STATIC_DRAW);

        // Update the View Matrix
        globalShader->SetUniformMatrix("u_ViewMatrix", vMatrix, warnMissingShaderUniforms);
        // Update the Projection Matrix
        globalShader->SetUniformMatrix("u_ProjectionMatrix", pMatrix, warnMissingShaderUniforms);
        // Update the combined ViewProjection Matrix
        globalShader->SetUniformMatrix("u_ViewProjectionMatrix", vpMatrix, warnMissingShaderUniforms);
        // Update the Time Uniform
        globalShader->SetUniformValue("u_Time", time, warnMissingShaderUniforms);
        
        globalMaterial->SetMaterialProperties(warnMissingShaderUniforms);
        //globalShader->SetLightUniforms(lightsUbo, MAX_LIGHTS);
        globalShader->SetLightUniformsRaw(lights);

        glBindVertexArray(globalMesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, globalMesh->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalMesh->ebo);
        // TEMPORARY WHILE WE FIGURE OUT WHAT'S WRONG WITH INSTANCED RENDERING
        for (Instance* i : instances) {
            if (!i->object->IsEnabled()) continue;
            if (i->extraAttribs.size() > 0) {
                globalShader->SetUniformVector("u_Color", i->extraAttribs.at(0));
                if (verbose) std::cout << "Setting color attribute of " << i->object->objectName << std::endl;
            }
            globalShader->SetUniformMatrix("u_ModelMatrix", i->object->GetGlobalTransform().GetModelMatrix(), warnMissingShaderUniforms);
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(globalMesh->elementCount), GL_UNSIGNED_INT, (void*)0);
        }
        // glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(globalMesh->elementCount), GL_UNSIGNED_INT, (void*)0, enabledInstances);
        glBindVertexArray(0);
    }
};

#endif