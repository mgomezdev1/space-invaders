#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <vector>
#include <glad/glad.h>
#include <SDL2/SDL.h>

#include "../gameObject.hpp"
#include "../shader.hpp"
#include "../glHelper.hpp"

class GLProgram;

class Light : public Component {
public:
    LightData data;

    Light(LightData lightData) {
        data = lightData;
    }

    void Start(GLProgram* program) { 
        program->AddLight(&this->data);
        //std::cout << "Registered new light: " << GetGameObject()->objectName << std::endl; 
    }
    void Destroy(GLProgram* program) {
        program->RemoveLight(&this->data);
    }
    void OnEnable(GLProgram* program) {
        data.active = true;
    }
    void OnDisable(GLProgram* program) {
        data.active = false;
    }
    void Update(GLProgram* program) { }
    void PreDraw(GLProgram* program) {
        Transform globalSpace = GetGameObject()->GetGlobalTransform();
        data.position = globalSpace.position;
    }
};

#endif