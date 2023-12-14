#ifndef LAYER_HPP
#define LAYER_HPP

#include <vector>
#include <algorithm>

#include "collider.hpp"
#include "collision.hpp"
#include "../gameObject.hpp"
#include "../extensions/collectionUtils.hpp"

bool ColliderComparer(Collider* c1, Collider* c2) {
    return c1->GetBounds().GetMinBound().x < c2->GetBounds().GetMinBound().x;
}

class CollisionLayer {
private:
    std::vector<CollisionLayer*> collisionable;
    std::vector<Collider*> layerColliders;

    ObjEventHandler<CollisionLayer, Component*> OnColliderDestroyedHandler;
    static void OnColliderDestroyedCallback(CollisionLayer* layer, Component* c) {
        layer->RemoveCollider(static_cast<Collider*>(c));
    }    
public:
    CollisionLayer() {
        OnColliderDestroyedHandler = ObjEventHandler<CollisionLayer, Component*>(this, OnColliderDestroyedCallback);
    }
    CollisionLayer* CollidesWith(CollisionLayer* other) {
        collisionable.push_back(other);
        return this;
    }
    CollisionLayer* AddCollider(Collider* collider) {
        layerColliders.push_back(collider);
        collider->OnDestroyed.AddListener(&OnColliderDestroyedHandler);
        return this;
    }
    CollisionLayer* RemoveCollider(Collider* collider) {
        Remove(layerColliders, collider);
    }
    void CollisionPrep() {
        if (layerColliders.size() < 2) return;
        std::sort(layerColliders.begin(), layerColliders.end(), ColliderComparer);
    }
    vector<Collision> CheckCollisions() {
        if (collisionable.size() == 0 || layerColliders.size() == 0) return {};
        for (CollisionLayer* otherLayer : collisionable) {
            if (otherLayer->layerColliders.size() == 0) continue;
            // Traverse through own colliders in axis order
            int leftIdx = 0;
            for (Collider* col : layerColliders) {
                if (!col->IsEnabled() || !col->GetGameObject()->IsEnabled()) continue;
                
                /* FASTER ALGORITHM BELOW, requires sorted bounds */
                float boundMin = col->GetBounds().GetMinBound().x; 
                float boundMax = col->GetBounds().GetMaxBound().x; 
                
                // Advance the leftIdx pointer up to the feasible range.
                while (leftIdx < otherLayer->layerColliders.size()) {
                    Collider* other = otherLayer->layerColliders.at(leftIdx);
                    if (!other->IsEnabled() || 
                        !other->GetGameObject()->IsEnabled() || 
                        other->GetBounds().GetMaxBound().x < boundMin) {
                        leftIdx++;
                        continue;
                    } else {
                        break;
                    }
                }
                // Check all colliders until they can't collide with the current collider
                for (int i = leftIdx; i < otherLayer->layerColliders.size(); ++i) {
                    Collider* other = otherLayer->layerColliders.at(i);
                    if (other->GetBounds().GetMinBound().x > boundMax) {
                        // No more colliders from the other layer can collide with this one
                        break;
                    }
                    if (!other->IsEnabled() || !other->GetGameObject()->IsEnabled()) {
                        continue;
                    }
                    // Standard case!
                    if (col->CollidesWith(other)) {
                        col->OnCollisionEnter.Invoke(col, other);
                        other->OnCollisionEnter.Invoke(other, col);
                        std::cout << col->GetGameObject()->objectName << " (" << col->GetBounds().ToString() << ") collided with " << other->GetGameObject()->objectName << " (" << other->GetBounds().ToString() << ")" << std::endl;
                    }
                }
            } 
        }
    }

};

#endif