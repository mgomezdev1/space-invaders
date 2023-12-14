#ifndef GLARE_HPP
#define GLARE_HPP

#include <string>

#include "../../lib/glHelper.hpp"
#include "../../lib/gameObject.hpp"
#include "../../lib/components/light.hpp"

class Glare : public GameObject {
private:
    float initialLifetime;
    float lifetime;
    Light* light;
public:
    Glare(std::string name, Transform transform, MeshHandle mesh, Material* material) 
    : GameObject(name, transform, mesh, material) {
        light = new Light(LightData({0,0,0},{1,1,1}));
        AddComponent(light);
        light->Start(GLProgram::Instance);
        Reset();
    }
    ~Glare() {
        
    }
    void Reset() {
        this->initialLifetime = RandomValue(1,1.2);
        this->lifetime = initialLifetime;
    }
    static glm::vec4 AttenuationCurve(float t) {
        return glm::vec4(0,0.05,0,0) / t + glm::vec4(0.05,0,0.01,0);
    }
    static glm::vec3 ColorCurve(float t) {
        return glm::vec3(1,1,1) * t + glm::vec3(0.5,0.3,0.1) * (1 - t);
    }
    virtual void Update(GLProgram* program) {
        light->data.attenuation = AttenuationCurve(lifetime / initialLifetime);
        light->data.color = ColorCurve(lifetime / initialLifetime);
        lifetime -= program->deltaTime;
        if (lifetime < 0) {
            this->SetEnabled(false, program);
        }
    }
};

#endif