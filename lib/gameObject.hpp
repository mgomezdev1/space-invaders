#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>
#include <functional>

#include "shader.hpp"
#include "geometry/mesh.hpp"
#include "extensions/event.hpp"

using namespace glm;
using namespace std;

class Transform {
protected:        
    // Where is our camera positioned
    // What direction is the camera looking
    glm::vec3 forwardVector;
    // Which direction is 'up' in our world
    // Generally this is constant, but if you wanted
    // to 'rock' or 'rattle' the camera you might play
    // with modifying this value.
    glm::vec3 upVector;
    glm::vec3 CalcRightVector() const {
        return glm::normalize(glm::cross(forwardVector, upVector));
    }
public:
    Event<Transform*, const vec3&, const vec3&> OnMoved;
    Event<Transform*, const vec3&, const vec3&> OnScaled;

    Transform(vec3 position = {0,0,0}, vec3 forward = {0,0,-1}, vec3 up = {0,1,0}, vec3 scale={1,1,1}) {
        this->position = position;
        this->scale = scale;
        SetForwardVector(forward);
        SetUpVector(up);
    }
    glm::vec3 position;
    glm::vec3 scale;
    void SetForwardVector(vec3 newVector, bool adjustUpVector = false) {
        forwardVector = glm::normalize(newVector);
        if (adjustUpVector) {
            upVector = glm::cross(CalcRightVector(), forwardVector);
        }
    }
    void SetUpVector(vec3 newVector, bool adjustForwardVector = false) {
        upVector = glm::normalize(newVector);
        if (adjustForwardVector) {
            forwardVector = glm::cross(CalcRightVector(), upVector);
        }
    }
    vec3 GetForwardVector() const {
        return forwardVector;
    }
    vec3 GetUpVector() const {
        return upVector;
    }

    // Rotation applied relative to the transform's existing rotation, rather than an axis
    void RotateUp(float radians, bool adjustUpVector = false) {
        glm::vec3 rightVector = CalcRightVector();
        SetForwardVector(glm::rotate(forwardVector,-radians,rightVector), adjustUpVector);
    }
    void RotateDown(float radians, bool adjustUpVector = false) {
        RotateUp(-radians, adjustUpVector);
    }
    void RotateRight(float radians, bool adjustUpVector = false) {
        SetForwardVector(glm::rotate(forwardVector,-radians,upVector), adjustUpVector);
    }
    void RotateLeft(float radians, bool adjustUpVector = false) {
        RotateRight(-radians, adjustUpVector);
    }

    bool IsUpsideDown() const {
        return upVector.y < 0;
    }

    // Rotation applied per-axis in the order Z-Y-X
    void Rotate(const vec3& eulerAngles) {
        if (eulerAngles.z != 0) RotateZ(eulerAngles.z);
        if (eulerAngles.y != 0) RotateY(eulerAngles.y);
        if (eulerAngles.x != 0) RotateX(eulerAngles.x);
    }
    void RotateX(float angle) {
        forwardVector = glm::rotateX(forwardVector, angle);
        upVector = glm::rotateX(upVector, angle);
    }
    void RotateY(float angle) {
        forwardVector = glm::rotateY(forwardVector, angle);
        upVector = glm::rotateY(upVector, angle);
    }
    void RotateZ(float angle) {
        forwardVector = glm::rotateZ(forwardVector, angle);
        upVector = glm::rotateZ(upVector, angle);
    }
    void SetRotation(const vec3& eulerAngles) {
        forwardVector = {0,0,-1};
        upVector = {0,1,0};
        Rotate(eulerAngles);
    }

    // Move the object in some given direction
    void Translate(const vec3& delta) {
        SetPosition(position + delta);
    }
    void SetPosition(const vec3& position) {
        vec3 oldPos = this->position;
        this->position = position;
        OnMoved.Invoke(this, oldPos, position);
    }
    vec3 GetPosition() const {
        return position;
    }
    void SetScale(const float scale) {
        SetScale(vec3(scale,scale,scale));
    }
    void SetScale(const vec3& scale) {
        vec3 oldScale = this->scale;
        this->scale = scale;
        OnScaled.Invoke(this, oldScale, scale);
    }
    vec3 GetScale() const {
        return scale;
    }
    void MoveForward(float distance) {
        Translate(forwardVector * distance);
    }
    void MoveBackward(float distance) {
        MoveForward(-distance);
    }
    void MoveRight(float distance) {
        auto rightVector = CalcRightVector();
        Translate(rightVector * distance);
    }
    void MoveLeft(float distance) {
        MoveRight(-distance);
    }
    void MoveUp(float distance) {
        Translate(upVector * distance);
    }
    void MoveDown(float distance) {
        MoveUp(-distance);
    }

    // Gets the model matrix by applying known transformations to an identity matrix.
    mat4 GetModelMatrix() const {
        mat4 translationMatrix = glm::translate(glm::mat4(), position);
        vec3 rightVector = CalcRightVector();
        // We need to correct the up vector to make sure we have an orthonormal basis
        vec3 upVectorCorrected = glm::cross(rightVector, forwardVector);
        mat4 rotationMatrix = mat4(
            vec4(rightVector.x, rightVector.y, rightVector.z, 0.0f),
            vec4(upVectorCorrected.x, upVectorCorrected.y, upVectorCorrected.z, 0.0f),
            vec4(-forwardVector.x, -forwardVector.y, -forwardVector.z, 0.0f),
            vec4(0.0f, 0.0f, 0.0f, 1.0f)
        );
        mat4 scaleMatrix = glm::scale(glm::mat4(), scale);
        mat4 model = translationMatrix * rotationMatrix * scaleMatrix;
        //cout << MatrixToStr(model) << endl;

        return model;
    }

    Transform ToParentSpace(const Transform& parent) {
        // Create a copy
        Transform result = Transform(*this);
        result.scale *= parent.scale;
        result.position += parent.position;
        mat3 parentRotation = mat3(parent.GetModelMatrix());
        result.SetForwardVector(parentRotation * result.forwardVector);
        result.SetUpVector(parentRotation * result.upVector);
        return result;
    }
};

// Forward declaration necessary to use these classes in Component
class GameObject;
class GLProgram;

class Component {
    friend class GameObject;
private:
    GameObject* gameObject = nullptr;
    bool enabled = true;
protected:
    virtual void OnEnable(GLProgram* context) { }
    virtual void OnDisable(GLProgram* context) { }
public:
    virtual void Start(GLProgram* context) { }
    virtual void Update(GLProgram* context) { }
    virtual void PreDraw(GLProgram* context) { }
    virtual void Draw(GLProgram* context) { }
    virtual void Destroy(GLProgram* context) { }

    Event<Component*> OnDestroyed;
    Event<Component*> OnEnabled;
    Event<Component*> OnDisabled;

    ~Component() {
        OnDestroyed.Invoke(this);
    }

    void SetEnabled(bool enabled, GLProgram* context) {
        if (this->enabled == enabled) return;
        this->enabled = enabled;
        if (enabled) {
            OnEnable(context);
            OnEnabled.Invoke(this);
        }
        else {
            OnDisable(context);
            OnDisabled.Invoke(this);
        }
    }
    bool IsEnabled() const {return enabled;}

    GameObject* GetGameObject() {
        return gameObject;
    }
};

// Forward declaration
class GLProgram;
class Material;

// A GameObject holds all the information necessary (post-vertex specification) to render a mesh
// It also exposes a Transform for ease of transformation of the object's model matrix.
class GameObject {
private:
    GameObject* parent;
    vector<GameObject*> children;
    bool enabled;
public:
    MeshHandle meshHandle;
    string objectName;
    Transform transform;
    Material* material;
    vector<Component*> components;
    bool isInstanced;
    GameObject(const string& name, const Transform& transform, const MeshHandle& meshHandle = MeshHandle(), Material* material = nullptr, bool isInstanced = false) {
        enabled = true;
        this->objectName = name;
        this->transform  = transform;
        this->meshHandle = meshHandle;
        this->material = material;
        this->isInstanced = isInstanced;
        this->parent = nullptr;
    }
    virtual void Destroy(GLProgram* context) {
        for (Component* c : components) {
            c->Destroy(context);
            delete c;
        }
    }
    virtual void SetEnabled(bool enabled, GLProgram* context) {
        this->enabled = enabled;
        for (Component* c : components) {
            c->SetEnabled(enabled, context);
        }
    }
    bool IsEnabled() const {
        return enabled;
    }
    template <typename T>
    T* GetComponent() {
        for (Component* c : components) {
            T* ptr = dynamic_cast<T*>(c);
            if (ptr != nullptr) {
                return ptr;
            }
        }
        return nullptr;
    }
    template <typename T>
    vector<T*> GetComponents() {
        vector<T*> result;
        for (Component* c : components) {
            T* ptr = dynamic_cast<T*>(c);
            if (ptr != nullptr) {
                result.push_back(ptr);
            }
        }
        return result;
    }
    template <typename T>
    T* GetComponentInChildren() {
        for (GameObject* go : children) {
            auto c = go->GetComponent<T>();
            if (c != nullptr) return c;
        }
        return nullptr;
    }
    template <typename T>
    vector<T*> GetComponentsInChildren() {
        vector<T*> result;
        for (GameObject* go : children) {
            vector<T*> vec = go->GetComponents<T>();
            AppendRange(result, vec);
        }
        return result;
    }

    void SetParent(GameObject* newParent) {
        if (parent != nullptr) {
            Remove(parent->children, this);
        }
        parent = newParent;
        newParent->children.push_back(this);
    }

    virtual void Start(GLProgram* context) {
        for (Component* c : components) {
            c->Start(context);
        }
    }
    virtual void Update(GLProgram* context) {
        for (Component* c : components) {
            if (!c->enabled) continue;
            c->Update(context);
        }
    }
    virtual void Draw(GLProgram* context) {
        for (Component* c : components) {
            if (!c->enabled) continue;
            c->Draw(context);
        }
    }
    virtual void PreDraw(GLProgram* context) {
        for (Component* c : components) {
            if (!c->enabled) continue;
            c->PreDraw(context);
        }
    }

    void AddComponent(Component* component) {
        components.push_back(component);
        component->gameObject = this;
    }

    Transform GetGlobalTransform() {
        if (parent == nullptr)
            return transform;
        else
            return transform.ToParentSpace(parent->GetGlobalTransform());
    }
};

#endif