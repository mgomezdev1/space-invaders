#ifndef SPACESHIP_HPP
#define SPACESHIP_HPP

#include <string>
#include <vector>

#include "../../lib/gameObject.hpp"
#include "../../lib/pools.hpp"
#include "../../lib/geometry/mesh.hpp"
#include "../../lib/shader.hpp"
#include "../../lib/input.hpp"
#include "../../lib/extensions/math.hpp"
#include "../../lib/collision/bounds.hpp"

#include "bullet.hpp"
#include "flame.hpp"

using namespace glm;

const Bounds PLAY_AREA({-9.0f, -5.5f, 0.0f}, {9.0f, 2.0f, 0.0f});
const float FIRING_COOLDOWN = 0.5f;
const float THRUST_COOLDOWN = 0.05f;
const float SPEED_FLAME_MULTIPLIER = 0.5f;
const glm::vec3 FLAME_OFFSET = {0,-0.5,0};
const glm::vec3 FLAME_OFFSET_SPREAD = {0.1,0.2,0.1};

const float MIN_THRUST_TIMER = -0.35f;

class SpaceShip : public GameObject {
    Collider* collider = nullptr;
    float maxSpeed = 5;
    float acceleration = 16;
    float maxAngle = 30 * pi<float>() / 2;

    GameObjectPool<Bullet>* bulletPool;
    GameObjectPool<Flame>* flamePool;

    float firingTimer = 0;
    float thrustTimer = 0;
public:
    vec3 velocity = {0,0,0};
    float angle = 0;

    Event<SpaceShip*> OnKilled;
    EventHandler<Collider*, Collider*> OnCollisionHandler = EventHandler<Collider*, Collider*>([](Collider* a, Collider* b){
        SpaceShip* ship = dynamic_cast<SpaceShip*>(a->GetGameObject());
        if (ship != nullptr) {
            ship->OnKilled.Invoke(ship);
        } else {
            std::cout << "Received collision with ship but failed to cast 'a' to SpaceShip*" << std::endl;
        }
    });

    SpaceShip(std::string name, Transform transform, MeshHandle mesh, Material* material, GameObjectPool<Bullet>* bulletPool, GameObjectPool<Flame>* flamePool, CollisionLayer* layer = nullptr) 
    : GameObject(name, transform, mesh, material) {
        collider = new BoxCollider(transform.GetPosition(), {0.65, 1.5, 1}, {0,0.75,0});
        this->AddComponent(collider);
        collider->Initialize();
        collider->OnCollisionEnter.AddListener(&OnCollisionHandler);

        this->bulletPool = bulletPool;
        this->flamePool = flamePool;
        if (layer != nullptr) {
            AttachToCollisionLayer(layer);
        }
        Reset();
    }
    void Reset() {
        transform.SetPosition({0,-4.5f,0});
    }
    
    void AttachToCollisionLayer(CollisionLayer* layer) {
        layer->AddCollider(collider);
    }

    virtual void HandleMotion(GLProgram* program) {
        vec3 motion = Input::GetAxes();
        motion.y = motion.z;
        motion.z = 0;
        vec3 velocityTarget = maxSpeed * SafeNormalize(motion);
        velocity = Step(velocity, velocityTarget, program->deltaTime * acceleration);
        float speed = length(velocity);
        if (speed > maxSpeed) {
            velocity = normalize(velocity) * maxSpeed;
            speed = maxSpeed;
        }
        vec3 newPosition = PLAY_AREA.SnapToBounds(transform.GetPosition() + velocity * program->deltaTime);
        transform.SetPosition(newPosition);
        float rotation = -velocity.x / maxSpeed;
        vec3 forward = vec3(sin(rotation),0,-cos(rotation));
        transform.SetForwardVector(forward);
    }

    virtual void HandleFiring(GLProgram* program) {
        firingTimer -= program->deltaTime;
        if ((!Input::IsDown(SDL_SCANCODE_SPACE) && !Input::mouseDown) || firingTimer > 0) return;
        firingTimer = FIRING_COOLDOWN;
        Bullet* newBullet = bulletPool->FetchUnused();
        newBullet->transform.SetPosition(transform.GetPosition());
        newBullet->Reset();
    }
    virtual void HandleThrust(GLProgram* program) {
        thrustTimer -= program->deltaTime * (1 + length(velocity) * SPEED_FLAME_MULTIPLIER);
        if (thrustTimer < MIN_THRUST_TIMER) thrustTimer = MIN_THRUST_TIMER;
        while (thrustTimer <= 0) {
            thrustTimer += THRUST_COOLDOWN;
            Flame* newFlame = flamePool->FetchUnused();
            newFlame->transform.SetPosition(transform.GetPosition() + FLAME_OFFSET + RandomValue(-1,1) * FLAME_OFFSET_SPREAD);
            newFlame->Reset();
        }
    }

    virtual void Update(GLProgram* program) {
        HandleMotion(program);
        HandleFiring(program);
        HandleThrust(program);
    }
};

#endif