#ifndef SCENE_H
#define SCENE_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "game_object.h"
#include "physics_world.h"
#include "game_UI.h"

class Scene
{
public:
    Scene();
    virtual ~Scene() = default;

    virtual void initialize() = 0;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(sf::Time deltaTime);
    virtual void render(sf::RenderWindow& window);

    void addGameObject(std::unique_ptr<GameObject> object);
    void setWindow(sf::RenderWindow* window);
    sf::RenderWindow* getWindow() const { return mWindow; }

    bool isCompleted() const;

protected:
    std::vector<std::unique_ptr<GameObject>> mGameObjects;
    PhysicsWorld mPhysicsWorld;
    sf::RenderWindow* mWindow;
    bool mIsCompleted;
    GameUI mGameUI;

    // New methods to update UI
    void updateUI(int projectilesLeft, int enemiesLeft, int currentLevel);
};


#endif