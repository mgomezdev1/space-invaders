#ifndef POOLS_HPP
#define POOLS_HPP

#include <string>
#include <functional>

#include "glHelper.hpp"
#include "gameObject.hpp"
#include "shader.hpp"

template <typename T>
T* InternalStandardGameObjectFactory(std::string name, int index, MeshHandle* mesh, Material* material){
    return new T(name + " " + to_string(index), Transform(), *mesh, material);
}

template <typename T>
std::function<T*(std::string, int, MeshHandle*, Material*)> StandardGameObjectFactory(InternalStandardGameObjectFactory<T>);

template <typename T>
class GameObjectPool {
private:
    std::vector<T*> pooledObjects;
    GLProgram* program;
    size_t activeIndex = 0;
    std::function<T*(std::string, int, MeshHandle*, Material*)> factory;
public:
    std::string name;
    MeshHandle* mesh;
    Material* material;
    GameObjectPool(std::string name, MeshHandle* mesh, Material* material, std::function<T*(std::string, int, MeshHandle*, Material*)> factory, GLProgram* program) {
        this->name = name;
        this->mesh = mesh;
        this->material = material;
        this->factory = factory;
        this->program = program;
    }
    std::vector<T*> GetObjects() {
        return pooledObjects;
    }
    T* FetchUnused(bool autoEnable = true) {
        //std::cout << "Fetching object from pool " << name << ", with " << pooledObjects.size() << " objects." << std::endl; 
        for (int i = activeIndex; i < activeIndex + pooledObjects.size(); ++i) {
            int idx = activeIndex % pooledObjects.size();
            T* obj = pooledObjects.at(idx);
            if (obj->IsEnabled()) continue;
            activeIndex = (idx + 1) % pooledObjects.size();
            if (autoEnable) obj->SetEnabled(true, program);
            return obj;
        }
        activeIndex = 0;
        return New();
    }
    T* New() {
        T* obj = factory(name, pooledObjects.size(), mesh, material);
        pooledObjects.push_back(obj);
        program->Instantiate(obj);
        return obj;
    }
};

class BasicGameObjectPool : GameObjectPool<GameObject> {
public:
    BasicGameObjectPool(std::string name, MeshHandle* mesh, Material* material, GLProgram* program) 
        :
        GameObjectPool<GameObject>(name, mesh, material, StandardGameObjectFactory<GameObject>, program)
    { }
};

#endif