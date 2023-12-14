#ifndef BLAST_HPP
#define BLAST_HPP

#include <string>

#include "../../lib/glHelper.hpp"
#include "../../lib/gameObject.hpp"

class BlastParticle : public GameObject {
private:
    float initialLifetime;
    float lifetime;
    vec3 initialVelocity;
    vec3 velocity;
    vec3 angularVelocity;
    vec3 initialScale;
    vec4 initialColor;
public:
    Instance* instance;
    BlastParticle(std::string name, Transform transform, MeshHandle mesh, Material* material) 
    : GameObject(name, transform, mesh, material) {
        this->instance = new Instance(this);
        this->instance->AddAttribute(vec4(1));
        Reset();
    }
    ~BlastParticle() {
        delete instance;
    }
    void Reset() {
        vec3 xyzRotation = RandomPointAround({0,0,0},1);
        transform.SetForwardVector(xyzRotation, false);
        transform.SetUpVector({0,1,0});
        this->initialLifetime = RandomValue(0.3,0.9);
        this->lifetime = initialLifetime;
        this->initialVelocity = RandomPointAround({0,1,0}, 0.5, 4);
        this->velocity = initialVelocity;
        this->angularVelocity = RandomPointAround({0,0,0}, 0, 12);
        this->initialScale = vec3(RandomValue(0.05f, 0.25f));
        this->initialColor = vec4(1,1,1,1);
        transform.SetScale(initialScale);
    }
    void SetColor(glm::vec4 color) {
        this->initialColor = color;
    }
    static glm::vec3 VelocityCurve(glm::vec3 v0, float t) {
        return v0 * (0.3f + 0.7f * t);
    }
    static glm::vec3 ScaleCurve(glm::vec3 s0, float t) {
        return s0 * t;
    }
    static glm::vec4 ColorCurve(glm::vec4 c0, float t) {
        return c0 * t + glm::vec4(0.8,0.8,0.8,1) * (1 - t);
    }
    virtual void Update(GLProgram* program) {
        transform.Translate(velocity * program->deltaTime);
        transform.SetScale(ScaleCurve(initialScale, lifetime / initialLifetime));
        transform.Rotate(angularVelocity * program->deltaTime);
        velocity = VelocityCurve(initialVelocity, lifetime / initialLifetime);
        instance->extraAttribs.at(0) = ColorCurve(initialColor, lifetime / initialLifetime);
        lifetime -= program->deltaTime;
        if (lifetime < 0) {
            this->SetEnabled(false, program);
        }
    }
};

#endif