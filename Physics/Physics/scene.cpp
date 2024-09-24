#include "scene.h"
#include "game_object.h"
#include <iostream>

Scene::Scene() : mIsCompleted(false), mWindow(nullptr) {}

void Scene::update(sf::Time deltaTime)
{
    mPhysicsWorld.update(deltaTime.asSeconds());

    for (auto it = mGameObjects.begin(); it != mGameObjects.end();)
    {
        if (*it == nullptr)
        {
            std::cerr << "Encountered null GameObject in update loop. Removing it.\n";
            it = mGameObjects.erase(it);
            continue;
        }

        if ((*it)->isMarkedForDeletion())
        {
            mPhysicsWorld.unregisterGameObject(it->get());
            it = mGameObjects.erase(it);
        }
        else {
            (*it)->update(deltaTime);
            ++it;
        }
    }
}

void Scene::addGameObject(std::unique_ptr<GameObject> gameObject)
{
    if (gameObject)
    {
        std::cout << "Adding new game object to scene" << std::endl;
        mPhysicsWorld.registerGameObject(std::move(gameObject));
        mGameObjects.push_back(std::move(gameObject));
    }
    else
    {
        std::cerr << "Attempted to add null game object to scene" << std::endl;
    }
}

void Scene::setWindow(sf::RenderWindow* window)
{
    mWindow = window;
    mGameUI.initialize(window);
}

void Scene::render(sf::RenderWindow& window)
{
    for (const auto& object : mGameObjects)
    {
        if (object)
        {
            object->render(window);
        }
        else
        {
            std::cerr << "Encountered null GameObject in render loop.\n";
        }
    }

    mGameUI.render(window);
}

void Scene::updateUI(int projectilesLeft, int enemiesLeft, int currentLevel)
{
    mGameUI.update(projectilesLeft, enemiesLeft, currentLevel);
}

void Scene::handleEvent(const sf::Event& event)
{
    // Default implementation (can be overridden in derived classes)
}

bool Scene::isCompleted() const
{
    return mIsCompleted;
}