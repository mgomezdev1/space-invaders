#include <iostream>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../lib/input.hpp"
#include "../../lib/shader.hpp"
#include "../../lib/geometry/mesh.hpp"
#include "../../lib/readers/objReader.hpp"
#include "../../lib/readers/ppmReader.hpp"
#include "../../lib/glHelper.hpp"
#include "../../lib/components/light.hpp"
#include "../../lib/geometry/bulk.hpp"
#include "../../lib/pools.hpp"

#include "../include/alien.hpp"
#include "../include/bullet.hpp"
#include "../include/spaceship.hpp"
#include "../include/flame.hpp"
#include "../include/blastParticle.hpp"
#include "../include/glare.hpp"

using namespace std;
using namespace glm;

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = SCREEN_WIDTH * 9/16;

SDL_GLContext glContext;
SDL_Window* glWindow;

GameObject* star;
SpaceShip* ship;
GameObjectPool<Alien>* alienPool;
GameObjectPool<Bullet>* bulletPool;
GameObjectPool<Flame>* flamePool;
GameObjectPool<BlastParticle>* blastPool;
GameObjectPool<Glare>* glarePool;

GameObject* g_defeatScreen;
GameObject* g_victoryScreen;

CollisionLayer g_playerBulletLayer;
CollisionLayer g_playerLayer;
CollisionLayer g_alienLayer;
std::vector<CollisionLayer*> g_collisionLayers;

InstancedRenderer alienRenderer;
InstancedRenderer fireRenderer;
InstancedRenderer bulletRenderer;
InstancedRenderer blastRenderer;

Shader* colliderShader;

// Unable to get functions to link
//Mix_Music* g_music = nullptr;
//Mix_Chunk* g_laserSfx = nullptr;
//Mix_Chunk* g_explosionSfx = nullptr;

const float ALIEN_SPAN_X = 14;
const float ALIEN_BASE_X = -ALIEN_SPAN_X / 2;
const float ALIEN_BASE_Y = 1.5;
const float ALIEN_SPAN_Y = 3.5; // maximum height above y = ALIEN_BASE
const glm::vec3 ALIEN_SCALE = {0.25,0.25,0.25};

const float BASE_ALIEN_SPEED = 0.8;
const float ALIEN_SPEED_INCREASE = 0.4;
float alienSpeed = 1;
const float ALIEN_VERTICAL_DISTANCE = 0.8;
float alienVerticalPeriod = 1;

const float UI_DELAY = 1;
float victoryTimer = -1;
float defeatTimer = -1;

int liveAlienCount = 0;
void ResetGame(GLProgram* program, bool tryUseExistingAliens = true, bool verbose = false) {
    vector<vector<unsigned char>> layout = Alien::ReadRandomLayout();
    liveAlienCount = 0;
    ship->SetEnabled(true, GLProgram::Instance);
    ship->Reset();
    g_victoryScreen->SetEnabled(false, program);
    g_defeatScreen ->SetEnabled(false, program);
    defeatTimer = -1;
    victoryTimer = -1;
    for (Alien* a : alienPool->GetObjects()) {
        a->SetEnabled(false, program);
    }
    int aliensPerCol = layout.size();
    if (aliensPerCol == 0) {
        throw std::runtime_error("Found 0 aliens per column, at least one alien must exist per column");
    }
    int aliensPerRow = layout.at(0).size();
    if (verbose) cout << "ALIEN LAYOUT SIZE: " << aliensPerRow << "x" << aliensPerCol << std::endl;
    
    float baseX = ALIEN_BASE_X;
    float baseY = ALIEN_BASE_Y + ALIEN_SPAN_Y;
    float deltaX; 
    float deltaY; 
    if (aliensPerRow <= 1) {
        baseX = ALIEN_BASE_X + ALIEN_SPAN_X / 2;
        deltaX = 0;
    } else {
        deltaX = (float)ALIEN_SPAN_X / (aliensPerRow - 1);
    }
    if (aliensPerCol <= 1) {
        baseY = ALIEN_BASE_Y + ALIEN_SPAN_Y / 2;
        deltaY = 0;
    } else {
        deltaY = (float)ALIEN_SPAN_Y / (aliensPerCol - 1);
    }

    int x, y;
    if (verbose) cout << "ALIEN LAYOUT:" << std::endl;
    for (y = 0; y < aliensPerCol; ++y) {
        auto row = layout.at(y); 
        for (x = 0; x < aliensPerRow; ++x) {
            if (verbose) cout << ((row.at(x) != 0) ? "#" : " ");
            if (row.at(x) == 1) {
                Alien* alien = nullptr;
                if (tryUseExistingAliens) {
                    alien = alienPool->FetchUnused();
                } else {
                    alien = alienPool->New();
                }
                alien->Reset();
                liveAlienCount++;
                alien->transform.SetPosition(glm::vec3(baseX + x * deltaX, baseY - y * deltaY, Alien::zOffset));
                alien->SetEnabled(true, program);
            }
        }
        if (verbose) cout << std::endl;
    }
    if (verbose) cout << "FINISHED ALIEN LAYOUT RESETTING:" << std::endl;
}
void SpawnAliens(GLProgram* program, bool verbose = false) {
    ResetGame(program, false, verbose);
}
void Victory() {
    if (g_victoryScreen->IsEnabled() || g_defeatScreen->IsEnabled()) return;
    g_victoryScreen->SetEnabled(true, GLProgram::Instance);
    alienSpeed += ALIEN_SPEED_INCREASE;
    alienVerticalPeriod = ALIEN_VERTICAL_DISTANCE / alienSpeed;
}
EventHandler<Alien*> OnAlienKilledHandler = EventHandler<Alien*>([](Alien* a){
    liveAlienCount--;
    int blastParticleCount = Random(50, 65);
    glm::vec3 position = a->transform.GetPosition();
    glm::vec4 alienColor = a->GetColor();
    Glare* g = glarePool->FetchUnused();
    g->Reset();
    g->transform.SetPosition(position + glm::vec3(0,0,2));
    for (int i = 0; i < blastParticleCount; i++) {
        BlastParticle* b = blastPool->FetchUnused();
        b->Reset();
        b->transform.SetPosition(position);
        b->SetColor(alienColor);
    }
    if (liveAlienCount <= 0 && victoryTimer <= 0) {
        victoryTimer = 1;
    }
});

void GameOver() {
    if (g_victoryScreen->IsEnabled() || g_defeatScreen->IsEnabled()) return;
    g_defeatScreen->SetEnabled(true, GLProgram::Instance);
    alienSpeed = BASE_ALIEN_SPEED;
    alienVerticalPeriod = ALIEN_VERTICAL_DISTANCE / alienSpeed;
}
void StartDefeat() {
    if (defeatTimer <= 0) {
        defeatTimer = UI_DELAY;
    }
}
vector<glm::vec4> shipExplosionColors = {
    {1,0.5,0,1}, {0,0.2,1,1}, {1,0.2,0,1}, {1,0.8,0.8,1}
};
EventHandler<SpaceShip*> OnShipKilledHandler = EventHandler<SpaceShip*>([](SpaceShip* s){
    StartDefeat();
    s->SetEnabled(false, GLProgram::Instance);
    Glare* g = glarePool->FetchUnused();
    g->Reset();
    g->transform.SetPosition(s->transform.GetPosition() + glm::vec3(0,0,2));
    for (int i = 0; i < 100; i++) {
        BlastParticle* b = blastPool->FetchUnused();
        b->Reset();
        b->transform.SetPosition(s->transform.GetPosition());
        b->SetColor(Sample(shipExplosionColors));
    }
});


/**
 * The main loop of the program, runs every frame
*/
void MainLoop(GLProgram* program) {
    float alienTimer = 0;
    bool drawColliders = false;
    bool aliensMoveRight = true;
    bool descending = false;

    while(program->programState != STATE_QUIT) {
        // Handle events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            program->HandleEvent(e, false);
            // Handle object Zoom
            if (e.type == SDL_MOUSEWHEEL) {
                const float SCROLL_FACTOR = 0.1f;
                program->camera.transform.MoveForward(e.wheel.y * SCROLL_FACTOR);
            }
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            if (e.type == SDL_KEYDOWN) {
                glm::vec3 delta = Input::GetAxes();
                delta.z = 0;
                switch(e.key.keysym.scancode) {
                    // Handle wireframe toggle
                    case SDL_SCANCODE_TAB:
                        program->wireframeRender = !program->wireframeRender;
                        break;
                    case SDL_SCANCODE_C:
                        drawColliders = !drawColliders;
                        break;
                    case SDL_SCANCODE_R:
                        ResetGame(program);
                        drawColliders = false;
                        aliensMoveRight = true;
                        descending = false;
                        break;
                }
                delta = SafeNormalize(delta);
            }
        }
        
        // Run the update function
        program->Update();

        //Move aliens to their new positions
        glm::vec3 motion = {0,0,0};
        if (descending) {
            motion = vec3(0,-1,0); 
        } else {
            motion = vec3(aliensMoveRight ? 1 : -1,0,0);
        }
        float time = MIN(program->deltaTime, 0.1);
        motion *= (time * alienSpeed);
        
        if (descending) {
            alienTimer -= time;
            if (alienTimer < 0) {
                descending = false;
                aliensMoveRight = !aliensMoveRight;
            }
        }

        for (Alien* a : alienPool->GetObjects()) {
            if (!a->IsEnabled()) continue;
            a->transform.Translate(motion);
            if (a->transform.position.y < PLAY_AREA.GetMinBound().y) {
                StartDefeat();
            }
            if (!descending) {
                if ((aliensMoveRight && a->transform.position.x > PLAY_AREA.GetMaxBound().x) ||
                   (!aliensMoveRight && a->transform.position.x < PLAY_AREA.GetMinBound().x)
                ) {
                    descending = true;
                    alienTimer = alienVerticalPeriod;
                }
            }
        }

        // Calculate collisions
        for (CollisionLayer* layer : g_collisionLayers) {
            layer->CollisionPrep();
        }
        g_playerBulletLayer.CheckCollisions();
        g_alienLayer.CheckCollisions();

        // Kick off the automated render pipeline, but don't swap the window buffer yet!
        program->Render(false, true, false);
        
        // Draw colliders
        if (drawColliders) {
            glm::mat4 vpMatrix = program->camera.GetVPMatrix(program->GetScreenSize());
            for (GameObject* obj : program->GetGameObjects()) {
                if (!obj->IsEnabled()) continue;
                for (Collider* col : obj->GetComponents<Collider>()) {
                    col->Render(colliderShader, vpMatrix);
                }
            }
        }

        if (victoryTimer > 0) {
            victoryTimer -= program->deltaTime;
            if (victoryTimer <= 0) {
                Victory();
            }
        } else if (defeatTimer > 0) {
            defeatTimer -= program->deltaTime;
            if (defeatTimer <= 0) {
                GameOver();
            }
        }

        program->SwapWindow();
        //std::cout << "Render Cycle Completed" << std::endl;
    }
}

MeshHandle g_alienMesh; 
Material* g_alienMat;

/**
* The entry point into our C++ programs.
*
* @return program status
*/
int main( int argc, char* args[] ){
    cout << "Show controls here...\n";
    cout << "Press ESC to quit.\n";

	// 1. Setup the graphics program
    GLProgram* program = (new GLProgram("Space Invaders", SCREEN_WIDTH, SCREEN_HEIGHT))->EnableLighting();
    program->camera.transform.SetPosition({0,0,10});
    program->camera.farPlane = 100;
    program->backgroundColor = {0.1,0.1,0.1,1};

    // Initialize audio mixer - UNABLE TO LINK LIBRARY
    /*
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }
    */
    cout << "Initialized program" << endl;

	// 2. Create our graphics pipeline
	// 	- At a minimum, this means the vertex and fragment shader
	Shader* unlitShader = program->BuildPipeline("./shaders/vert_unlit.glsl", "./shaders/frag_unlit.glsl");
	Shader* litShader   = program->BuildPipeline("./shaders/vert_lit.glsl", "./shaders/frag_lit.glsl")->EnableLighting();
    Shader* bgShader    = program->BuildPipeline("./shaders/vert_bg.glsl", "./shaders/frag_bg.glsl");
    Shader* fireShader  = program->BuildPipeline("./shaders/vert_unlit_instanced_colored.glsl", "./shaders/frag_unlit_instanced_colored.glsl");
    Shader* alienShader = program->BuildPipeline("./shaders/vert_lit_instanced.glsl", "./shaders/frag_lit_instanced.glsl")->EnableLighting();
    Shader* bulletShader= program->BuildPipeline("./shaders/vert_unlit_instanced.glsl", "./shaders/frag_unlit_instanced.glsl");
    Shader* uiShader    = program->BuildPipeline("./shaders/vert_ui.glsl", "./shaders/frag_unlit.glsl");
    colliderShader      = program->BuildPipeline("./shaders/vert_collider.glsl", "./shaders/frag_collider.glsl");

    cout << "Built Shader Pipelines" << endl;

    g_playerBulletLayer.CollidesWith(&g_alienLayer);
    g_alienLayer.CollidesWith(&g_playerLayer);
    g_collisionLayers.push_back(&g_playerLayer);
    g_collisionLayers.push_back(&g_playerBulletLayer);
    g_collisionLayers.push_back(&g_alienLayer);

    //  - Create images
    Image img = Image::Solid(Pixel(255,255,255));
    Texture2D blank = program->LoadTexture(&img);
    img = Image::Solid(Pixel(128,128,255));
    Texture2D blankNormal = program->LoadTexture(&img);

    ObjData alienObjData = ObjReader::ReadObj("./media/objects/alien.obj").at(0);
    g_alienMesh = program->LoadMesh(alienObjData.mesh.get());
    // IMPORTANT: Change to "alienShader" once instanced rendering is fixed
    g_alienMat = program->LoadRawMtl(alienObjData.materialData, litShader, blank, blankNormal);

    ObjData shipObjData = ObjReader::ReadObj("./media/objects/rocket.obj").at(0);
    MeshHandle shipMesh = program->LoadMesh(shipObjData.mesh.get());
    Material* shipMat = program->LoadRawMtl(shipObjData.materialData, litShader, blank, blankNormal);

    ObjData bulletObjData = ObjReader::ReadObj("./media/objects/bullet.obj").at(0);
    MeshHandle bulletMesh = program->LoadMesh(bulletObjData.mesh.get());
    // IMPORTANT: Change to "bulletShader" once instanced rendering is fixed
    Material* bulletMat = program->LoadRawMtl(bulletObjData.materialData, unlitShader, blank, blankNormal);

    ObjData starObjData = ObjReader::ReadObj("./media/objects/star.obj").at(0);
    MeshHandle starMesh = program->LoadMesh(starObjData.mesh.get());
    Material* starMat = program->LoadRawMtl(starObjData.materialData, unlitShader, blank, blankNormal);
    
    ObjData fireObjData = ObjReader::ReadObj("./media/objects/flame.obj").at(0);
    MeshHandle fireMesh = program->LoadMesh(fireObjData.mesh.get());
    // IMPORTANT: Change to "fireShader" once instanced rendering is fixed
    Material* fireMat = program->LoadRawMtl(fireObjData.materialData, unlitShader, blank, blankNormal);

    ObjData blastObjData = ObjReader::ReadObj("./media/objects/blast.obj").at(0);
    MeshHandle blastMesh = program->LoadMesh(blastObjData.mesh.get());
    // IMPORTANT: Change to "fireShader" once instanced rendering is fixed
    Material* blastMat = program->LoadRawMtl(blastObjData.materialData, unlitShader, blank, blankNormal);

    ObjData bgObjData = ObjReader::ReadObj("./media/objects/space.obj").at(0);
    MeshHandle bgMesh = program->LoadMesh(bgObjData.mesh.get());
    Material* bgMat = program->LoadRawMtl(bgObjData.materialData, bgShader, blank, blankNormal);

    RawMtl victoryRawMat = MtlReader::ReadMtl("./media/objects/ui_victory.mtl").at(0);
    Material* victoryMat = program->LoadRawMtl(victoryRawMat, uiShader, blank, blankNormal);
    RawMtl defeatRawMat = MtlReader::ReadMtl("./media/objects/ui_defeat.mtl").at(0);
    Material* defeatMat  = program->LoadRawMtl(defeatRawMat, uiShader, blank, blankNormal);

    std::cout << "Loaded Objects and Materials" << std::endl;
    
    alienRenderer = InstancedRenderer::WithNewBuffer(&g_alienMesh, g_alienMat, 1);
    fireRenderer  = InstancedRenderer::WithNewBuffer(&fireMesh, fireMat, 1);
    bulletRenderer = InstancedRenderer::WithNewBuffer(&bulletMesh, bulletMat);
    blastRenderer = InstancedRenderer::WithNewBuffer(&blastMesh, blastMat, 1);
    program->Instantiate(&alienRenderer);
    program->Instantiate(&fireRenderer);
    program->Instantiate(&bulletRenderer);
    program->Instantiate(&blastRenderer);

    std::cout << "Built Bulk Renderers" << std::endl;

    alienPool = new GameObjectPool<Alien>("Alien", &g_alienMesh, g_alienMat, [](std::string name, int id, MeshHandle* mesh, Material* mat) {
        Alien* a = new Alien("Alien", Transform({0,0,0}, {0,0,-1}, {0,1,0}, ALIEN_SCALE), g_alienMesh, g_alienMat, &g_alienLayer);
        a->OnKilled.AddListener(&OnAlienKilledHandler);
        alienRenderer.AddInstance(a->instance);
        return a;
    }, program);
    bulletPool = new GameObjectPool<Bullet>("Bullet", &bulletMesh, bulletMat, [](std::string name, int id, MeshHandle* mesh, Material* mat) {
        Bullet* b = new Bullet(name, Transform({0,0,0}, {0,0,1}, {0,1,0}, {0.1,0.1,0.1}), *mesh, mat, &g_playerBulletLayer);
        bulletRenderer.AddInstance(b->instance);
        b->Start(GLProgram::Instance);
        return b;
    }, program);
    flamePool  = new GameObjectPool<Flame>("Flame", &fireMesh, fireMat, [](std::string name, int id, MeshHandle* mesh, Material* mat) {
        Flame* f = new Flame(name, Transform({0,0,0}, {0,0,1}, {0,1,0}, {0.4,0.4,0.4}), *mesh, mat);
        fireRenderer.AddInstance(f->instance);
        f->Start(GLProgram::Instance);
        return f;
    }, program);
    blastPool  = new GameObjectPool<BlastParticle>("Blast", &blastMesh, blastMat, [](std::string name, int id, MeshHandle* mesh, Material* mat) {
        BlastParticle* b = new BlastParticle(name, Transform({0,0,0}, {0,0,1}, {0,1,0}, {0.4,0.4,0.4}), *mesh, mat);
        blastRenderer.AddInstance(b->instance);
        b->Start(GLProgram::Instance);
        return b;
    }, program);
    glarePool = new GameObjectPool<Glare>("Glare", &blastMesh, blastMat, [](std::string name, int id, MeshHandle* mesh, Material* mat) {
        Glare* g = new Glare(name, Transform({0,0,0}, {0,0,1}, {0,1,0}, {0,0,0}), *mesh, mat);
        g->Start(GLProgram::Instance);
        return g;
    }, program);

    ship = new SpaceShip(shipObjData.name, Transform({0,0,0}, {0,0,1}, {0,1,0}, {0.25, 0.25, 0.25}), shipMesh, shipMat, bulletPool, flamePool, &g_playerLayer);
    program->Instantiate(ship);
    ship->OnKilled.AddListener(&OnShipKilledHandler);
    
    GameObject* background = new GameObject("Background", Transform(), bgMesh, bgMat);
    program->Instantiate(background);
    g_victoryScreen = new GameObject("Victory", Transform(), bgMesh, victoryMat);
    g_defeatScreen  = new GameObject("Defeat",  Transform(), bgMesh, defeatMat);
    program->Instantiate(g_victoryScreen);
    program->Instantiate(g_defeatScreen);
    g_victoryScreen->SetEnabled(false, program);
    g_defeatScreen ->SetEnabled(false, program);

    star = new GameObject(starObjData.name, Transform({10, 3.5, 10}, {-1,0,0}, {0,1,0}, {3,3,3}), starMesh, starMat);
    Light* light = new Light(LightData(vec3(0),vec3(1),vec4(0.2f,0.005f,0.0005f,0)));
    star->AddComponent(light);
    program->Instantiate(star);

    std::cout << "Spawned Ship and Star" << std::endl;
    SpawnAliens(program, false);

    program->warnMissingShaderUniforms = true;

    std::cout << "Spawned Aliens" << std::endl;

    // Load audio - UNABLE TO LINK LIBRARY
    // g_music = Mix_LoadMUS("./media/audio/music.mp3");
    // g_laserSfx = Mix_LoadWAV("./media/audio/laser.wav");
    // g_explosionSfx = Mix_LoadWAV("./media/audio/explosion.wav");

    // Mix_PlayMusic(g_music, -1);

    // 4. Call the main application loop
    program->Start();
	MainLoop(program);

	// 5. Call the cleanup function when our program terminates
	delete alienPool;
    delete bulletPool;
    delete flamePool;
    delete program;
    // WAS UNABLE TO GET THE LIBRARY TO LINK PROPERLY
    //Mix_FreeChunk(g_laserSfx);
    //Mix_FreeChunk(g_explosionSfx);
    //Mix_FreeMusic(g_music);
    //Mix_Quit();

	return 0;
}
