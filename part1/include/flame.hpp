#ifndef FLAME_HPP
#define FLAME_HPP

#include <string>

#include "../../lib/glHelper.hpp"
#include "../../lib/gameObject.hpp"

class Flame : public GameObject {
private:
    float initialLifetime;
    float lifetime;
    vec3 initialVelocity;
    vec3 velocity;
    vec3 angularVelocity;
    vec3 initialScale;
public:
    Instance* instance;
    Flame(std::string name, Transform transform, MeshHandle mesh, Material* material) 
    : GameObject(name, transform, mesh, material) {
        this->instance = new Instance(this);
        this->instance->AddAttribute(vec4(1));
        Reset();
    }
    ~Flame() {
        delete instance;
    }
    void Reset() {
        vec2 xyRotation = RandomPointAround({0,0},1);
        transform.SetForwardVector({xyRotation.x,0,xyRotation.y}, false);
        transform.SetUpVector({0,1,0});
        this->initialLifetime = RandomValue(0.5,1.5);
        this->lifetime = initialLifetime;
        float initialSpeed = RandomValue(2, 5);
        vec2 xyOffset = RandomPointAround({0,0},0,1);
        this->initialVelocity = normalize(glm::vec3(xyOffset.x, -3, xyOffset.y)) * initialSpeed;
        this->velocity = initialVelocity;
        this->angularVelocity = normalize(RandomPointAround({0,0,0}, 0, 8));
        this->initialScale = vec3(RandomValue(0.15f, 0.35f));
        transform.SetScale(initialScale);
    }
    static glm::vec3 VelocityCurve(glm::vec3 v0, float t) {
        return v0 * (0.3f + 0.7f * t);
    }
    static glm::vec3 ScaleCurve(glm::vec3 s0, float t) {
        return s0 * t;
    }
    static glm::vec4 ColorCurve(float t) {
        return glm::vec4(1,1,1,1) * t + glm::vec4(0.4,0.2,1,1) * (1 - t);
    }
    virtual void Update(GLProgram* program) {
        transform.Translate(velocity * program->deltaTime);
        transform.SetScale(ScaleCurve(initialScale, lifetime / initialLifetime));
        transform.Rotate(angularVelocity * program->deltaTime);
        velocity = VelocityCurve(initialVelocity, lifetime / initialLifetime);
        instance->extraAttribs.at(0) = ColorCurve(lifetime / initialLifetime);
        lifetime -= program->deltaTime;
        if (lifetime < 0) {
            this->SetEnabled(false, program);
        }
    }
};

#endif