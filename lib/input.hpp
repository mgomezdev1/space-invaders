#ifndef INPUT_HPP
#define INPUT_HPP

#include <glm/glm.hpp>
#include <SDL2/SDL.h> 

#include "extensions/math.hpp"
#include "extensions/event.hpp"

class Input {
public:
    static SDL_Scancode FORWARD;
    static SDL_Scancode BACKWARD;
    static SDL_Scancode RIGHT;
    static SDL_Scancode LEFT;
    static SDL_Scancode UP;
    static SDL_Scancode DOWN;

    static bool mouseDown;
    static bool altMouseDown;

    static Event<float, float> OnMouseDown;
    static Event<float, float> OnAltMouseDown;
    static Event<float, float> OnMouseUp;
    static Event<float, float> OnAltMouseUp;
    static EventHandler<float,float> MouseDownHandler;
    static EventHandler<float,float> AltMouseDownHandler;
    static EventHandler<float,float> MouseUpHandler;
    static EventHandler<float,float> AltMouseUpHandler;

    // X = left-right, Y = down-up, Z = backwards-forwards
    static glm::vec3 GetAxes(bool normalized = false) {
        // Retrieve keyboard state
        const Uint8 *state = SDL_GetKeyboardState(NULL);

        glm::vec3 motionVector;
        if (state[FORWARD]) {
            motionVector.z += 1;
        }
        if (state[BACKWARD]) {
            motionVector.z -= 1;
        }
        if (state[LEFT]) {
            motionVector.x -= 1;
        }
        if (state[RIGHT]) {
            motionVector.x += 1;
        }
        if (state[UP]) {
            motionVector.y += 1;
        }
        if (state[DOWN]) {
            motionVector.y -= 1;
        }

        return SafeNormalize(motionVector);
    }

    static bool IsDown(SDL_Scancode key) {
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        return state[key];
    }

    static void ProcessMouseEvents(SDL_Event event) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            std::cout << "MOUSE DOWN" << std::endl;
            int x,y;
            SDL_GetGlobalMouseState(&x, &y);
            if (event.button.button == SDL_BUTTON_LEFT) {
                Input::OnMouseDown.Invoke(x, y);
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                Input::OnAltMouseDown.Invoke(x, y);
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            std::cout << "MOUSE UP" << std::endl;
            int x,y;
            SDL_GetGlobalMouseState(&x, &y);
            if (event.button.button == SDL_BUTTON_LEFT) {
                Input::OnMouseUp.Invoke(x, y);
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                Input::OnAltMouseUp.Invoke(x, y);
            }
        }
    }

    static void Initialize() {
        Input::OnMouseDown.AddListener(&Input::MouseDownHandler);
        Input::OnAltMouseDown.AddListener(&Input::AltMouseDownHandler);
        Input::OnMouseUp.AddListener(&Input::MouseUpHandler);
        Input::OnAltMouseUp.AddListener(&Input::AltMouseUpHandler);
    }
};

SDL_Scancode Input::FORWARD  {SDL_SCANCODE_W};
SDL_Scancode Input::BACKWARD {SDL_SCANCODE_S};
SDL_Scancode Input::RIGHT    {SDL_SCANCODE_D};
SDL_Scancode Input::LEFT     {SDL_SCANCODE_A};
SDL_Scancode Input::UP       {SDL_SCANCODE_SPACE};
SDL_Scancode Input::DOWN     {SDL_SCANCODE_LSHIFT};

bool Input::mouseDown     {false};
bool Input::altMouseDown  {false};

Event<float, float> Input::OnMouseDown {Event<float,float>()};
Event<float, float> Input::OnAltMouseDown {Event<float,float>()};
Event<float, float> Input::OnMouseUp {Event<float,float>()};
Event<float, float> Input::OnAltMouseUp {Event<float,float>()};

EventHandler<float, float> Input::MouseDownHandler {EventHandler<float, float>([](float x, float y){
    Input::mouseDown = true;
})};
EventHandler<float, float> Input::AltMouseDownHandler {EventHandler<float, float>([](float x, float y){
    Input::altMouseDown = true;
})};
EventHandler<float, float> Input::MouseUpHandler {EventHandler<float, float>([](float x, float y){
    Input::mouseDown = false;
})};
EventHandler<float, float> Input::AltMouseUpHandler {EventHandler<float, float>([](float x, float y){
    Input::altMouseDown = false;
})};

#endif