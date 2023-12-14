#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "../gameObject.hpp"
#include "bounds.hpp"
#include "../extensions/math.hpp"

class Collider : public Component {
private:
    bool started = false;
    GLuint vao;
    GLuint vbo;
    unsigned int renderPointCount = 0;
    bool renderDirty = true;
public:
    virtual Bounds GetBounds() = 0;
    virtual Collider* Translate(glm::vec3 delta) = 0;
    virtual Collider* SetOrigin(glm::vec3 delta) = 0;
    virtual Collider* SetOffset(glm::vec3 delta) = 0;
    virtual bool Contains(glm::vec3 point) = 0;
    virtual std::vector<glm::vec3> GetProbePoints() = 0;

    Event<Collider*, Collider*> OnCollisionEnter;

    static void OnMoveCallback(Collider* instance, Transform* t, const glm::vec3& oldPos, const glm::vec3& newPos) {
        instance->SetOrigin(newPos);
    }
    ObjEventHandler<Collider, Transform*, const glm::vec3&, const glm::vec3&> OnMoveHandler;
    
    // Called only by instantiated subclasses
    Collider() {
        OnMoveHandler = ObjEventHandler<Collider, Transform*, const glm::vec3&, const glm::vec3&>(this, OnMoveCallback);
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }
    ~Collider() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    bool CollidesWith(Collider* other) {
        for (glm::vec3 p : other->GetProbePoints()) {
            if (Contains(p)) return true;
        }
        for (glm::vec3 p : GetProbePoints()) {
            if (other->Contains(p)) return true;
        }
        return false;
    }

    virtual void Initialize() {
        if (started) return;
        if (this->GetGameObject() != nullptr) {
            this->GetGameObject()->transform.OnMoved.AddListener(&OnMoveHandler);
            started = true;
        }
    }

    void Detach() {
        if (this->GetGameObject() != nullptr) {
            this->GetGameObject()->transform.OnMoved.RemoveListener(&OnMoveHandler);
        }
    }

    virtual std::vector<glm::vec3> GetRenderPoints() {
        std::vector<glm::vec3> result = GetProbePoints();
        if (result.size() == 0) return {};
        result.push_back(result.at(0));
        return result;
    }
    virtual void MarkRenderDirty() {
        renderDirty = true;
    }
    virtual void RecalculateRenderData() {
        std::vector<glm::vec3> points = GetRenderPoints();
        if (points.size() == 0) return;
        //std::cout << "Rendering collider with " << points.size() << " points." << std::endl;
        //std::cout << VectorToStr(rawPoints) << std::endl;
        size_t rawSize = sizeof(glm::vec3) * points.size();
        renderPointCount = points.size();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // Update the vertex locations
        glBufferData(GL_ARRAY_BUFFER, rawSize, points.data(), GL_DYNAMIC_DRAW);
        // Rebind attribute pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        renderDirty = false;
    }

    virtual void Render(Shader* colliderShader, glm::mat4 viewProjectionMatrix, glm::vec3 color = {0.2,1,0.2}) {
        if (renderDirty) {
            RecalculateRenderData();
        }
        colliderShader->Use();

        glBindVertexArray(vao);

        colliderShader->SetUniformMatrix("u_MVPMatrix", viewProjectionMatrix, true);
        colliderShader->SetUniformVector3("u_Color", color, true);

        glDrawArrays(GL_LINE_STRIP, 0, renderPointCount);
        glBindVertexArray(0);
    }
};

class CenteredCollider : public Collider {
protected:
    glm::vec3 origin = {0,0,0};
    glm::vec3 offset = {0,0,0};
    glm::vec3 center = {0,0,0};
    float scale;
public:
    virtual Bounds GetBounds() = 0;
    Collider* Translate(glm::vec3 delta) {
        return SetOrigin(this->origin + delta);
    }
    Collider* SetOrigin(glm::vec3 origin) {
        this->origin = origin;
        this->center = origin + offset;
        return this;
    }
    Collider* SetOffset(glm::vec3 offset) {
        this->offset = offset;
        this->center = origin + offset;
        return this;
    }
    virtual bool Contains(glm::vec3 point) = 0;
    virtual std::vector<glm::vec3> GetProbePoints() = 0;
};

class SphereCollider : public CenteredCollider {
protected:
    Bounds bounds;
    float radius;
    float squareRadius;

    virtual void RecalculateBounds() {
        this->bounds = Bounds(center - glm::vec3(radius), center + glm::vec3(radius));
        MarkRenderDirty();
    }
public:
    SphereCollider(glm::vec3 origin, float radius, glm::vec3 offset = {0,0,0}) {
        this->origin = origin;
        this->offset = offset;
        this->center = origin + offset;
        this->radius = radius;
        this->squareRadius = radius * radius;
        RecalculateBounds();
    }
    Bounds GetBounds() {
        return bounds;
    }
    Collider* SetOrigin(glm::vec3 origin) {
        CenteredCollider::SetOrigin(origin);
        RecalculateBounds();
        return this;
    }
    Collider* Scale(float scale) {
        return SetRadius(this->radius * scale);
    }
    Collider* SetRadius(float radius) {
        this->radius = radius;
        this->squareRadius = radius * radius;
        RecalculateBounds();
        return this;
    }
    bool Contains(glm::vec3 point) {
        return SquareMagnitude(point - center) < squareRadius;
    }
    inline std::vector<glm::vec3> GetProbePoints() {
        return {
            bounds.GetRelativePoint(0.5,0.5, 1 ),
            bounds.GetRelativePoint(0.5,0.5, 0 ),
            bounds.GetRelativePoint(0.5, 1 ,0.5),
            bounds.GetRelativePoint(0.5, 0 ,0.5),
            bounds.GetRelativePoint( 1 ,0.5,0.5),
            bounds.GetRelativePoint( 0 ,0.5,0.5)
        };
    }

    virtual std::vector<glm::vec3> GetRenderPoints() {
        float r22 = (sqrt(2)/4) + 0.5; // root of 2 over 2 (-1,1) passed into the space (0,1)
        float r2c = 1 - r22; // complement
        return {
            bounds.GetRelativePoint(0.5,0.5, 1 ), // XZ plane
            bounds.GetRelativePoint(r22,0.5,r22),
            bounds.GetRelativePoint( 1 ,0.5,0.5),
            bounds.GetRelativePoint(r22,0.5,r2c),
            bounds.GetRelativePoint(0.5,0.5, 0 ),
            bounds.GetRelativePoint(r2c,0.5,r2c),
            bounds.GetRelativePoint( 0 ,0.5,0.5),
            bounds.GetRelativePoint(r2c,0.5,r22),
            bounds.GetRelativePoint(0.5,0.5, 1 ), // YZ plane
            bounds.GetRelativePoint(0.5,r22,r22),
            bounds.GetRelativePoint(0.5, 1 ,0.5),
            bounds.GetRelativePoint(0.5,r22,r2c),
            bounds.GetRelativePoint(0.5,0.5, 0 ),
            bounds.GetRelativePoint(0.5,r2c,r2c),
            bounds.GetRelativePoint(0.5, 0 ,0.5),
            bounds.GetRelativePoint(0.5,r2c,r22),
            bounds.GetRelativePoint(0.5,0.5, 1 ),
            bounds.GetRelativePoint(0.5,r22,r22), // Transition to XY plane (duplicated vertex)
            bounds.GetRelativePoint(0.5, 1 ,0.5), // XY plane
            bounds.GetRelativePoint(r22,r22,0.5),
            bounds.GetRelativePoint( 1 ,0.5,0.5),
            bounds.GetRelativePoint(r22,r2c,0.5),
            bounds.GetRelativePoint(0.5, 0 ,0.5),
            bounds.GetRelativePoint(r2c,r2c,0.5),
            bounds.GetRelativePoint( 0 ,0.5,0.5),
            bounds.GetRelativePoint(r2c,r22,0.5),
            bounds.GetRelativePoint(0.5, 1 ,0.5)
        };
    }
};

class EllipsoidCollider : public SphereCollider {
protected:
    glm::vec3 scale = {1,1,1};

    virtual void RecalculateBounds() {
        this->bounds = Bounds(center - scale * radius, center + scale * radius);
        MarkRenderDirty();
    }
public:
    EllipsoidCollider(glm::vec3 origin, float radius, glm::vec3 scale, glm::vec3 offset = {0,0,0}) : SphereCollider(origin, radius, offset) {
        SetScale(scale);
    }
    EllipsoidCollider(glm::vec3 origin, glm::vec3 scale, glm::vec3 offset = {0,0,0}) : SphereCollider(origin, 1, offset) {
        SetScale(scale);
    }
    void SetScale(glm::vec3 scale) {
        this->scale = scale;
        RecalculateBounds();
    }
    glm::vec3 GetScale() {
        return scale;
    }
    bool Contains(glm::vec3 point) {
        return SquareMagnitude((point - center) / scale) < squareRadius;
    }
};

class BoxCollider : public CenteredCollider {
protected:
    Bounds bounds;
    glm::vec3 size;

    void RecalculateBounds() {
        this->bounds = Bounds(center - size * 0.5f, center + size * 0.5f);
        MarkRenderDirty();
    }
public:
    BoxCollider(glm::vec3 origin, glm::vec3 size, glm::vec3 offset = {0,0,0}) {
        this->origin = origin;
        this->offset = offset;
        this->center = origin + offset;
        this->size = size;
        RecalculateBounds();
    }
    Bounds GetBounds() {
        return bounds;
    }
    Collider* SetOrigin(glm::vec3 center) {
        CenteredCollider::SetOrigin(center);
        RecalculateBounds();
        return this;
    }
    Collider* SetOffset(glm::vec3 offset) {
        CenteredCollider::SetOffset(offset);
        RecalculateBounds();
        return this;
    }
    Collider* Scale(glm::vec3 scale) {
        return SetSize(this->size * scale);
    }
    Collider* SetSize(glm::vec3 size) {
        this->size = size;
        RecalculateBounds();
        return this;
    }
    bool Contains(glm::vec3 point) {
        return this->bounds.Contains(point);
    }
    inline std::vector<glm::vec3> GetProbePoints() {
        return {
            bounds.GetRelativePoint(0,0,0),
            bounds.GetRelativePoint(0,0,1),
            bounds.GetRelativePoint(0,1,0),
            bounds.GetRelativePoint(0,1,1),
            bounds.GetRelativePoint(1,0,0),
            bounds.GetRelativePoint(1,0,1),
            bounds.GetRelativePoint(1,1,0),
            bounds.GetRelativePoint(1,1,1)
        };
    }

    virtual std::vector<glm::vec3> GetRenderPoints() {
        return {
            bounds.GetRelativePoint(0,0,0), // LEFT square
            bounds.GetRelativePoint(0,0,1),
            bounds.GetRelativePoint(0,1,1),
            bounds.GetRelativePoint(0,1,0),
            bounds.GetRelativePoint(1,1,0), // RIGHT square
            bounds.GetRelativePoint(1,1,1),
            bounds.GetRelativePoint(1,0,1),
            bounds.GetRelativePoint(1,0,0), 
            bounds.GetRelativePoint(1,0,1), // BOTTOM square
            bounds.GetRelativePoint(1,0,0),
            bounds.GetRelativePoint(0,0,0),
            bounds.GetRelativePoint(0,0,1),
            bounds.GetRelativePoint(0,1,1), // FRONT square
            bounds.GetRelativePoint(1,1,1),
            bounds.GetRelativePoint(1,0,1),
            bounds.GetRelativePoint(0,0,1),
            bounds.GetRelativePoint(0,1,1), // TOP square
            bounds.GetRelativePoint(0,1,0),
            bounds.GetRelativePoint(1,1,0),
            bounds.GetRelativePoint(1,1,1),
            bounds.GetRelativePoint(1,1,0), // BACK square
            bounds.GetRelativePoint(1,0,0),
            bounds.GetRelativePoint(0,0,0),
            bounds.GetRelativePoint(0,1,0)
        };
    }
};

#endif