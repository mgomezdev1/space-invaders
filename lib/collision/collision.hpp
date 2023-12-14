#ifndef COLLISION_HPP
#define COLLISION_HPP

#include <glm/glm.hpp>

#include "collider.hpp"

struct Collision {
    Collider* collider1;
    Collider* collider2;
    Collision(Collider* col1, Collider* col2) {
        this->collider1 = col1;
        this->collider2 = col2;
    }
};

#endif