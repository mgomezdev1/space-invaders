#ifndef BULLET_HPP
#define BULLET_HPP

#include <string>

#include "../../lib/glHelper.hpp"
#include "../../lib/gameObject.hpp"
#include "../../lib/collision/collider.hpp"
#include "../../lib/collision/layer.hpp"
#include "../../lib/collision/bounds.hpp"
#include "../../lib/extensions/math.hpp"

const glm::vec3 BULLET_COLLIDER_SIZE = {0.10f,0.45f,0.10f};
const Bounds BULLET_BOUNDS = Bounds({-16,-16,-16},{16,16,16});

class Bullet : public GameObject {
private:
    const glm::vec3 VELOCITY = {0, 10, 0};
    float spinSpeed = PI * 8;
    Light* light;
public:
    Collider* collider = nullptr;
    Instance* instance = nullptr;
    Bullet(std::string name, Transform transform, MeshHandle mesh, Material* material, CollisionLayer* layer = nullptr) 
    : GameObject(name, transform, mesh, material) {
        collider = new EllipsoidCollider(transform.GetPosition(), BULLET_COLLIDER_SIZE);
        this->AddComponent(collider);
        light = new Light(LightData({0,0,0},{0.5,0.5,1},{1,0.3f,0.05f,0.0f},0.0f,3.0f,3.0f));
        this->AddComponent(light);
        collider->Initialize();
        this->instance = new Instance(this);
        Reset();
        if (layer != nullptr) {
            AttachToCollisionLayer(layer);
        }
    }
    ~Bullet() {
        delete instance;
    }
    void Reset() {
        spinSpeed = RandomValue(4, 10) * PI * RandomSign();
    }
    static Transform BuildBulletTransform(glm::vec3 position, glm::vec3 firingDirection = {0,1,0}) {
        return Transform(position, firingDirection, {0,0,-1});
    }
    void AttachToCollisionLayer(CollisionLayer* layer) {
        layer->AddCollider(collider);
    }
    virtual void Update(GLProgram* program) {
        transform.Translate(VELOCITY * program->deltaTime);
        transform.RotateY(spinSpeed * program->deltaTime);
        if (!BULLET_BOUNDS.Contains(transform.position)) {
            SetEnabled(false, program);
        }
    }
};

#endif