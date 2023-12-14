#ifndef GL_HELPER_HPP
#define GL_HELPER_HPP

// Third Party Libraries

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

// Standard Libraries

#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_set>

// External files

#include "shader.hpp"
#include "gameObject.hpp"
#include "geometry/mesh.hpp"
#include "geometry/vertex.hpp"
#include "input.hpp"
#include "extensions/math.hpp"
#include "readers/ppmReader.hpp"
#include "readers/mtlReader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "geometry/bulk.hpp"

using namespace std;
using namespace glm;

#define MIN(x,y) x < y ? x : y
#define MAX(x,y) x > y ? x : y

#define MAX_LIGHTS 50

// vvvvvvvvvvvvvvvvvvv Error Handling Routines vvvvvvvvvvvvvvv
static void GLClearAllErrors(){
    while(glGetError() != GL_NO_ERROR){
    }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line){
    while(GLenum error = glGetError()) {
        std::cout << "OpenGL Error: " << error 
                  << "\tLine: " << line 
                  << "\tfunction: " << function << std::endl;
        return true;
    }
    return false;
} 

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);
// ^^^^^^^^^^^^^^^^^^^ Error Handling Routines ^^^^^^^^^^^^^^^

struct InitializationResult {
    SDL_GLContext context;
    SDL_Window* window;
};

void InitializeProgram(const std::string& title, int screenWidth, int screenHeight, SDL_Window*& window, SDL_GLContext& context){
	// Initialize SDL
    cout << "Initializing program" << endl;
    if(SDL_Init(SDL_INIT_VIDEO)< 0){
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }
    
    // Setup the OpenGL Context
    // Use OpenGL 4.1 core or greater
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    // We want to request a double buffer for smooth updating.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create an application window using OpenGL that supports SDL
    window = SDL_CreateWindow( title.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        screenWidth,
        screenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN 
    );

    // Check if Window did not create.
    if( window == nullptr ){
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Create an OpenGL Graphics Context
    context = SDL_GL_CreateContext( window );
    if( context == nullptr){
        std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Initialize GLAD Library
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
        std::cout << "glad did not initialize" << std::endl;
        exit(1);
    }

    // Initialize input events
    Input::Initialize();
}

/**
* The last function called in the program
* This functions responsibility is to destroy any global
* objects in which we have create dmemory.
*
* @return void
*/
void CleanUp(SDL_Window* window, const vector<GLuint>& vbos, const vector<GLuint>& vaos, const vector<Shader*>& builtShaders, const vector<GLuint>& textures){
	//Destroy our SDL2 Window
	SDL_DestroyWindow(window);
	window = nullptr;

    // Delete our OpenGL Objects
    glDeleteBuffers(vbos.size(), vbos.data());
    glDeleteVertexArrays(vaos.size(), vaos.data());
    glDeleteTextures(textures.size(), textures.data());
    
	// Delete our Graphics pipeline
    for (Shader* shader : builtShaders) {
        shader->Destroy();
        delete shader;
    }

	//Quit SDL subsystems
	SDL_Quit();
}

typedef int ProgramState;
ProgramState STATE_RUNNING = 0;
ProgramState STATE_PAUSED = 1;
ProgramState STATE_QUIT = 2;

class GLProgram {
private:
    vector<GLuint> buffers;
    vector<GLuint> vaos;
    vector<Shader*> builtShaders;
    vector<LightData*> lights;
    GLuint lightsUbo;
    vector<GameObject*> gameObjects;
    vector<Texture2D*> textures;
    vector<Material*> materials;
    vector<InstancedRenderer*> instancedRenderers;
    Shader* defaultShader;
    int screenX;
    int screenY;
    chrono::system_clock::time_point lastFrameTime;
    chrono::system_clock::time_point initialTime;
    SDL_GLContext context;
    SDL_Window* window;
    GLenum regularDrawMode = GL_FILL;

    vector<GLuint> GetTextureHandles() {
        vector<GLuint> result; 
        for (Texture2D* tex : textures) {
            result.push_back(tex->GetHandle());
        }
        return result;
    }
public:
    Camera camera;
    ProgramState programState;
    // We allow static access to the program to use it as a "Singleton"
    static GLProgram* Instance;

    float deltaTime = 0.00000001f;
    float time = 0.00000001f;
    vec2 mouse;

    bool wireframeRender = false;
    bool warnMissingShaderUniforms = false;
    
    vec4 backgroundColor = {0.3f, 0.0f, 0.75f, 1.0f};

    GLProgram(const std::string& title, int screenWidth, int screenHeight) {
        // Initialize the GL program with a few given parameters and store the window and context
        InitializeProgram(title, screenWidth, screenHeight, window, context);
        
        GLProgram::Instance = this;
        screenX = screenWidth;
        screenY = screenHeight;
        programState = STATE_RUNNING;
        mouse = GetScreenSize() * 0.5f;

        lastFrameTime, initialTime = chrono::system_clock::now();
    }
    ~GLProgram() {
        CleanUp(window, buffers, vaos, builtShaders, GetTextureHandles());
        for (GameObject* go : gameObjects) {
            go->Destroy(this);
            delete go;
        }
        for (Material* mat : materials) {
            delete mat;
        }
    }
    vec2 GetScreenSize() const {
        return vec2(screenX, screenY);
    }

    // Builds the graphics pipeline from a vertex and fragment shader path.
    Shader* BuildPipeline(const std::string& vertPath, const std::string& fragPath) {
        string vertSource = LoadShaderAsString(vertPath);
        string fragSource = LoadShaderAsString(fragPath);
        Shader* shader = new Shader(CreateShaderProgram(vertSource,fragSource));
        builtShaders.push_back(shader);
        return shader;
    }
    void SetDefaultShader(Shader* shader) {
        defaultShader = shader;
    }
    vector<Shader*> GetLoadedShaders() const {
        return builtShaders;
    }
    Shader* GetDefaultShader() const {
        return defaultShader;
    }

    // Register a buffer as being in use. This is only used for automatic buffer deletion during cleanup.
    void RegisterBuffer(GLuint bufferHandle) {
        buffers.push_back(bufferHandle);
    }
    // Register a VAO as being in use. This is only used for automatic buffer deletion during cleanup.
    void RegisterVAO(GLuint vaoHandle) {
        vaos.push_back(vaoHandle);
    }

    GLProgram* EnableLighting() {
        glGenBuffers(1, &lightsUbo);
        RegisterBuffer(lightsUbo);
        return this;
    }

    //Handles generic events on queue
    void HandleEvent(const SDL_Event& e, bool allowCameraMove = true) {
        // If users posts an event to quit
        // An example is hitting the "x" in the corner of the window.
        if(e.type == SDL_QUIT){
            std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            programState = STATE_QUIT;
        } else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
            std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            programState = STATE_QUIT;
        } else if(e.type == SDL_MOUSEMOTION){
            // Capture the change in the mouse position
            mouse.x+=e.motion.xrel;
            mouse.y+=e.motion.yrel;
            //std::cout << "Moved Mouse To (" << mouse.x << ", " << mouse.y << ")" << endl;
            if (allowCameraMove) camera.MouseLook(mouse);
        } else {
            Input::ProcessMouseEvents(e);
        }
    }

    void Start() {
        for (auto go : gameObjects) {
            cout << "Starting " << go->objectName << endl;
            go->Start(this);
        }
    }

    // runs once per frame
    void Update() {
        //cout << "Update Started" << endl;
        // Update our DeltaTime
        auto now = chrono::system_clock::now();
        chrono::duration<float> elapsedSeconds = now - lastFrameTime;
        deltaTime = elapsedSeconds.count();
        chrono::duration<float> elapsedSinceStart = now - initialTime;
        time = elapsedSinceStart.count(); 
        lastFrameTime = now;

        // keep mouse in the center of the window, hidden
        //SDL_WarpMouseInWindow(window,screenSize.x/2,screenSize.y/2);
        SDL_SetRelativeMouseMode(SDL_TRUE);

        // Update all components
        for (GameObject* go : GetGameObjects()) {
            if (!go->IsEnabled()) continue;
            // std::cout << "Updating " << go->objectName << "." << std::endl;
            go->Update(this);
        }
    }

    // Simulates camera motion based on default inputs
    void DefaultCameraMove(float cameraSpeed = 1.0f, bool alignCameraMotionWithAxes = false) {
        vec3 axes = Input::GetAxes(true);
        if (alignCameraMotionWithAxes) {
            vec3 forwardComponent = SafeNormalize(vec3(camera.GetViewXDirection(), 0, camera.GetViewZDirection()));
            vec3 rightComponent = vec3(-forwardComponent.z, 0, forwardComponent.x);
            vec3 upComponent = vec3(0, 1, 0);
            if (camera.transform.IsUpsideDown()) {
                rightComponent = -rightComponent;
                upComponent = -upComponent;
            }
            axes = SafeNormalize(axes.x * rightComponent + axes.y * upComponent + axes.z * forwardComponent);
            axes *= (deltaTime * cameraSpeed);
            camera.transform.Translate(axes);
            return;
        }

        camera.transform.MoveRight(axes.x);
        camera.transform.MoveUp(axes.y);    
        camera.transform.MoveForward(axes.z);    
    }

    // ##########################
    // # MESH RENDERING SECTION #
    // ##########################
    // Gets the number of bytes contained in a standard vertex with a given set of embedded data
    inline size_t GetAttributeSizes(MeshAttributeFlags flags) {
        return sizeof(GL_FLOAT) * (
            3 + // Position data (invariant)
            ((flags & MESH_UV_DATA) ? 1 : 0) * 2 + // Texture coords data
            ((flags & MESH_NORMAL_DATA) ? 1 : 0) * 3 + // Vertex normal data
            ((flags & MESH_COLOR_DATA) ? 1 : 0) * 4 + // Raw vertex color data
            ((flags & MESH_TANGENT_DATA) ? 1 : 0) * 3 // Texture map tangent space data
        );
    }

    // Loads mesh data into the program and provides a handle that references the mesh
    MeshHandle LoadMesh(const IMesh* mesh, MeshAttributeFlags attribFlags = MESH_BASIC_AND_COLOR_DATA) {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;

        size_t attribSize = GetAttributeSizes(attribFlags);

        // Vertex Arrays Object (VAO) Setup
        glGenVertexArrays(1, &vao);
        RegisterVAO(vao);
        // We bind (i.e. select) to the Vertex Array Object (VAO) that we want to work withn.
        glBindVertexArray(vao);
        
        // Vertex Buffer Object (VBO) creation
        glGenBuffers(1, &vbo);
        RegisterBuffer(vbo);
        
        vector<GLfloat> vertices = (*mesh).GetArrayBuffer(attribFlags);
        //cout << VectorToStr(vertices) << endl;
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, // Kind of buffer we are working with 
                                      // (e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER)
            vertices.size() * sizeof(GLfloat), 	// Size of data in bytes
            vertices.data(),          // Raw array of data
            GL_STATIC_DRAW);          // How we intend to use the data

        glGenBuffers(1, &ebo);
        RegisterBuffer(ebo);

        vector<GLuint> elements = (*mesh).GetElementArrayBuffer();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            elements.size() * sizeof(GLuint),
            elements.data(),
            GL_STATIC_DRAW
        );

        // Position information (x,y,z)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)0);

        size_t attribCounter = 0;
        size_t offsetCounter = 3;
        // UV information (u,v)
        if (attribFlags & MESH_UV_DATA) {
            //cout << "Enabling attributes for UV with offset " << offsetCounter << ", attribID " << attribCounter + 1 << endl;
            glEnableVertexAttribArray(++attribCounter);
            glVertexAttribPointer(attribCounter, 2, GL_FLOAT, GL_FALSE, attribSize, (GLvoid*)(sizeof(GLfloat)*offsetCounter));
            offsetCounter += 2;
        }

        // Normal information (nx,ny,nz)
        if (attribFlags & MESH_NORMAL_DATA) {
            //cout << "Enabling attributes for normal with offset " << offsetCounter << ", attribID " << attribCounter + 1<< endl;
            glEnableVertexAttribArray(++attribCounter);
            glVertexAttribPointer(attribCounter, 3, GL_FLOAT, GL_FALSE, attribSize, (GLvoid*)(sizeof(GLfloat)*offsetCounter));
            offsetCounter += 3;
        }

        // Color information (r,g,b,a)
        if (attribFlags & MESH_COLOR_DATA) {
            //cout << "Enabling attributes for color with offset " << offsetCounter << ", attribID " << attribCounter + 1 << endl;
            glEnableVertexAttribArray(++attribCounter);
            glVertexAttribPointer(attribCounter, 4, GL_FLOAT, GL_FALSE, attribSize, (GLvoid*)(sizeof(GLfloat)*offsetCounter));
            offsetCounter += 4;
        }

        // Tangent information (tx, ty, tz)
        if (attribFlags & MESH_TANGENT_DATA) {
            //cout << "Enabling attributes for tangent with offset " << offsetCounter << ", attribID " << attribCounter + 1 << endl;
            glEnableVertexAttribArray(++attribCounter);
            glVertexAttribPointer(attribCounter, 3, GL_FLOAT, GL_FALSE, attribSize, (GLvoid*)(sizeof(GLfloat)*offsetCounter));
            offsetCounter += 3;
        } 

        // Unbind our currently bound buffers
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        // Disable any attributes we opened in our Vertex Attribute Array,
        // as we do not want to leave them open. 
        for (int i = 0; i < attribCounter; i++) {
            glDisableVertexAttribArray(i);
        }

        return MeshHandle(vao, vbo, ebo, elements.size());
    }

    Texture2D LoadTexture(const Image* image, GLenum wrapMode=GL_REPEAT, GLenum minFilter=GL_LINEAR_MIPMAP_LINEAR, GLenum magFilter=GL_LINEAR, bool invertY = true, bool invertX = false) {
        GLuint handle;
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);
        // Set wrapping and filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        // Push the image pixel data to the GPU
        vector<unsigned char> rawData;
        image->Dump(rawData, invertY, invertX);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, rawData.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        // Unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);
        return Texture2D(handle, image->width, image->height);
    }

    // This will load a material from its description using relevant files
    // The program will delete the pointer on completion
    Material* LoadRawMtl(const RawMtl& mtlData, Shader* shader, const Texture2D& blankTexture, const Texture2D& defaultNormalMap, bool invertY = true, bool invertX = false,
        GLenum wrapMode=GL_REPEAT, GLenum minFilter=GL_LINEAR_MIPMAP_LINEAR, GLenum magFilter=GL_LINEAR) {
        unordered_map<string, Texture2D> texturesByFile;
        texturesByFile[""] = blankTexture;
        auto files = mtlData.GetFileNames();
        for (string s : files) {
            // If there is no file to read, do nothing
            if (s == "") continue;
            // If we have already read the file, do nothing
            if (texturesByFile.find(s) != texturesByFile.end()) continue;
            // If we find a PPM file, read it and dump its data into a Texture
            if (EndsWith(s,".ppm")) {
                Image img = PpmReader::ReadPpm(s);
                //cout << img.ToString() << endl;
                texturesByFile[s] = LoadTexture(&img, wrapMode, minFilter, magFilter, invertY, invertX);
            } else {
                std::cerr << "Unable to read texture file " << s << ", no decoder for its format is implemented." << endl;
            }
        }

        FullMaterial* result = new FullMaterial(shader, vec4(1), texturesByFile[mtlData.normalMapFile],
            ToVec4(mtlData.ambient, 1), texturesByFile[mtlData.ambientMapFile],
            ToVec4(mtlData.diffuse, 1), texturesByFile[mtlData.diffuseMapFile],
            ToVec4(mtlData.specular, 1), texturesByFile[mtlData.specularMapFile],
            mtlData.glossiness, texturesByFile[mtlData.glossinessMapFile],
            ToVec4(mtlData.emissive, 1), texturesByFile[mtlData.emissiveMapFile],
            mtlData.dissolve, texturesByFile[mtlData.dissolveMapFile],
            mtlData.refractiveIndex, mtlData.illumMode
        );

        if (mtlData.normalMapFile == "") result->SetNormalMap(defaultNormalMap);

        materials.push_back(result);
        return result;
    }

    // This will add the material to the program's context, allowing it to handle its lifetime without the user's input
    void RegisterExternalMaterial(Material* mat) {
        materials.push_back(mat);
    }

    // Provides a blank slate in the frame buffer and sets some global uniforms.
    void PreDraw() {
        // Disable depth test and face culling.
        glEnable(GL_DEPTH_TEST); // NOTE: Need to enable DEPTH Test
        //glDisable(GL_CULL_FACE);
        
        // Set the polygon mode
        glPolygonMode(GL_FRONT_AND_BACK,wireframeRender ? GL_LINE : regularDrawMode);

        // Initialize clear color
        // This is the background of the screen.
        glViewport(0, 0, screenX, screenY);
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);

        for (GameObject* go : GetGameObjects()) {
            if (!go->IsEnabled()) continue;
            go->PreDraw(this);
        }

        LoadLightData(lights);

        //Clear color buffer and Depth Buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }

    // Registering an object queues it for rendering on Render()
    void Instantiate(GameObject* gameObject) {
        std::cout << "Instantiating " << gameObject->objectName << "." << std::endl;
        gameObjects.push_back(gameObject);
    }
    vector<GameObject*> GetGameObjects() const {
        return gameObjects;
    }

    // Registering bulk objects to automatically render them
    void Instantiate(InstancedRenderer* renderer) {
        instancedRenderers.push_back(renderer);
    }

    void AddLight(LightData* lightData) {
        //cout << "Light added!" << endl;
        lights.push_back(lightData);
    }
    void RemoveLight(LightData* lightData) {
        Remove(lights, lightData);
    }
    void LoadLightData(vector<LightData*> lightData) {
        LightData* rawData = new LightData[MAX_LIGHTS];
        int limit = MIN(lightData.size(), MAX_LIGHTS);
        for (int i = 0; i < limit; i++) {
            rawData[i] = *lightData[i];
        }
        glBindBuffer(GL_UNIFORM_BUFFER, lightsUbo);
        glBufferData(GL_UNIFORM_BUFFER, 
            MAX_LIGHTS*sizeof(LightData), 
            rawData, 
            GL_STATIC_DRAW
        );
        int numLights = lightData.size();
        glBufferSubData(GL_UNIFORM_BUFFER, 
            MAX_LIGHTS * sizeof(LightData),
            sizeof(GLuint),
            &numLights
        );
        delete[] rawData;
    }
    void RefreshLightData(vector<LightData*> lightData, int start, int end) {
        if (start >= MAX_LIGHTS) return;
        int limit = MIN(end, MAX_LIGHTS) - start;
        LightData* rawData = new LightData[limit];
        for (int i = 0; i < limit; i++) {
            rawData[i] = *lightData[start+i];
        }
        glBindBuffer(GL_UNIFORM_BUFFER, lightsUbo);
        glBufferSubData(GL_UNIFORM_BUFFER,
            start * sizeof(LightData),
            limit * sizeof(LightData),
            rawData
        );
        delete[] rawData;
    }

    // Draw one specific gameObject, for optimization reasons, we precompute view, projection, and vpMatrices.
    void Draw(GameObject& gameObject, const mat4& vMatrix, const mat4& pMatrix, const mat4& vpMatrix) {
        // Use the gameObject's specific shader, or the default if it's not set (set to 0).
        Shader* goShader = gameObject.material->shader->GetHandle() == 0 ? defaultShader : gameObject.material->shader;
        goShader->Use();

        //std::cout << "Using shader " << goShader->GetHandle() << ".";

        // Update the View Matrix
        goShader->SetUniformMatrix("u_ViewMatrix", vMatrix, warnMissingShaderUniforms);
        // Update the Projection Matrix
        goShader->SetUniformMatrix("u_ProjectionMatrix", pMatrix, warnMissingShaderUniforms);
        // Update the combined ViewProjection Matrix
        goShader->SetUniformMatrix("u_ViewProjectionMatrix", vpMatrix, warnMissingShaderUniforms);
        // Update the Time Uniform
        goShader->SetUniformValue("u_Time", time, warnMissingShaderUniforms);
        //cout << "Drawing gameObject " << gameObject.objectName << ", with " << gameObject.meshHandle.elementCount << " elements" << endl;
        goShader->SetUniformMatrix("u_ModelMatrix", gameObject.transform.GetModelMatrix(), warnMissingShaderUniforms);

        //cout << "Setting material properties for: " << gameObject.objectName << endl;
        gameObject.material->SetMaterialProperties(warnMissingShaderUniforms);

        //goShader->SetLightUniforms(lightsUbo, MAX_LIGHTS);
        goShader->SetLightUniformsRaw(lights);
        
        //std::cout << "Set all uniforms for rendering object " << gameObject.objectName << ".";

        glBindVertexArray(gameObject.meshHandle.vao);
        glBindBuffer(GL_ARRAY_BUFFER, gameObject.meshHandle.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gameObject.meshHandle.ebo);

        glDrawElements(GL_TRIANGLES, gameObject.meshHandle.elementCount, GL_UNSIGNED_INT, (void*)0);
        
        gameObject.Draw(this);
    }

    // Kicks off the entire pipeline automatically, using internal program values and registered GameObjects.
    void Render(bool verbose = false, bool drawInstancedWithRenderers = true, bool swapWindow = true) {
        if (verbose) cout << "Started Render" << endl;
        PreDraw();
        if (verbose) cout << "Completed Predraw" << endl;
        mat4 vMatrix = camera.GetViewMatrix();
        mat4 pMatrix = camera.GetProjectionMatrix(GetScreenSize());
        mat4 vpMatrix = camera.GetVPMatrix(GetScreenSize());
        for (int i = 0; i < gameObjects.size(); i++) {
            if (verbose) cout << "Drawing " << gameObjects.at(i)->objectName << "...";
            auto go = gameObjects.at(i);
            // Ensure object is enabled
            if (!go->IsEnabled()) continue;
            if (go->isInstanced && drawInstancedWithRenderers) continue;
            Draw(*(gameObjects.at(i)), vMatrix, pMatrix, vpMatrix);
            if (verbose) cout << "Drawn." << endl;
        }
        if (drawInstancedWithRenderers) {
            for (int i = 0; i < instancedRenderers.size(); i++) {
                instancedRenderers.at(i)->Draw(vMatrix, pMatrix, vpMatrix, lights, time, warnMissingShaderUniforms, verbose);
                //std::cout << "Stepped outside of instanced rendering" << std::endl;
            }
        }
        if (verbose) cout << "Completed Draw" << endl;
        glUseProgram(0);
        if (swapWindow) {
            SwapWindow();
            if (verbose) cout << "Swapped Display Buffer" << endl;
        }
    }
    void SwapWindow() {
        SDL_GL_SwapWindow(window);
    }
    
    SDL_Window* GetWindow() const {return window;}

    void SetDrawMode(GLenum drawMode) {
        regularDrawMode = drawMode;
    }
};

GLProgram* GLProgram::Instance {nullptr};

#endif
