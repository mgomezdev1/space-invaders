#ifndef ALIEN_HPP
#define ALIEN_HPP

#include <fstream>
#include <sstream>

#include "../../lib/gameObject.hpp"
#include "../../lib/collision/layer.hpp"
#include "../../lib/geometry/bulk.hpp"
#include "../../lib/extensions/collectionUtils.hpp"
#include "../../lib/glHelper.hpp"

#include "bullet.hpp"

class Alien : public GameObject {
    Collider* collider = nullptr;
    static int alienCount;
    static std::vector<glm::vec4> colors;

    const float ALIEN_COLLIDER_RADIUS = 0.3f;
    static void CollisionCallback(Collider* a, Collider* b) {
        Alien* alien = dynamic_cast<Alien*>(a->GetGameObject());
        Bullet* bullet = dynamic_cast<Bullet*>(b->GetGameObject());
        if (alien != nullptr && bullet != nullptr) {
            a->GetGameObject()->SetEnabled(false, GLProgram::Instance);
            b->GetGameObject()->SetEnabled(false, GLProgram::Instance);
            alien->OnKilled.Invoke(alien);
        }
    }
    EventHandler<Collider*, Collider*> CollisionHandler = EventHandler<Collider*,Collider*>(CollisionCallback);
public:
    Instance* instance = nullptr;
    Event<Alien*> OnKilled;

    static float zOffset;

    Alien(std::string name, Transform transform, MeshHandle mesh, Material* material, CollisionLayer* layer = nullptr) 
    : GameObject(name, transform, mesh, material) {
        collider = new SphereCollider(transform.GetPosition(), ALIEN_COLLIDER_RADIUS, {0,0,-zOffset});
        collider->OnCollisionEnter.AddListener(&CollisionHandler);
        this->AddComponent(collider);
        collider->Initialize();
        if (layer != nullptr) {
            AttachToCollisionLayer(layer);
        }
        instance = (new Instance(this))->AddAttribute(vec4(1));
        Reset();
    }
    ~Alien() {
        delete instance;
    }
    
    void AttachToCollisionLayer(CollisionLayer* layer) {
        layer->AddCollider(collider);
    }
    void SetColor(glm::vec4 newColor) {
        instance->extraAttribs.at(0) = newColor;
    }
    glm::vec4 GetColor() {
        return instance->extraAttribs.at(0);
    }
    void SetRandomColor() {
        SetColor(Sample(colors));
    }
    void Reset() {
        SetRandomColor();
    }

    virtual void Update(GLProgram* program) {
        
    }

    static vector<vector<vector<unsigned char>>> ReadLayouts() {
        //std::cout << "Reading alien layouts" << std::endl;
        ifstream layoutFile("./media/data/alien_layouts.txt");
        vector<vector<vector<unsigned char>>> result;

        std::string line;
        int i = -1;
        int x, y = -1;
        while (std::getline(layoutFile, line)) {
            //std::cout << line << std::endl;
            std::istringstream iss(line);
            if (y <= 0) {
                iss >> y >> x;
                //std::cout << "Creating new layout of size " << x << "x" << y << std::endl;
                if (y == 0) break;
                result.push_back(vector<vector<unsigned char>>());
                ++i;
                continue;
            }
            y--;
            int n;
            vector<unsigned char> row;
            for (int i = 0; i < x; i++) {
                iss >> n;
                row.push_back(n);
            }
            result.at(i).push_back(row);
        }
        //std::cout << "Finished reading alien layouts with size " << result.size() << std::endl;

        return result;
    }

    static vector<vector<unsigned char>> ReadRandomLayout(bool verbose = false) {
        auto layouts = ReadLayouts();
        if (verbose) { 
            for (auto l : layouts) {
                std::cout << "Layout: " << std::endl;
                for (auto row : l) {
                    for (unsigned char c : row) {
                        std::cout << (c != 0 ? to_string(c) : " ");
                    }
                    std::cout << std::endl;
                }
            }
        }
        return Sample(layouts);
    }
};

float Alien::zOffset {-0.25f};
std::vector<glm::vec4> Alien::colors {
    {
        {1,0.2,0.2,1},
        {0.2,1,0.2,1},
        {0,0.5,1,1},
        {0.5,0,1,1},
        {1,0.5,1,1}
    }
};

#endif