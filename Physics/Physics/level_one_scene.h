#ifndef LEVEL_ONE_SCENE_H
#define LEVEL_ONE_SCENE_H

#include "scene.h"
#include "projectile.h"
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <vector>
#include <array>

class LevelOneScene : public Scene
{
public:
    LevelOneScene();
    void initialize() override;
    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;
    std::unique_ptr<sf::RectangleShape> mGroundShape;

    void addProjectile(std::unique_ptr<Projectile> projectile);
    void drawDebugInfo(sf::RenderWindow& window);
    bool checkCollision(GameObject* obj1, GameObject* obj2);
    bool isProjectileOffScreen(const std::unique_ptr<Projectile>& projectile) const;
    sf::Font mDebugFont;

private:
    void createGround();
    void resetProjectile();
    void createBlocks();
    void createEnemies();
    void checkLevelCompletion();
    void printAllBodies();
    void updateTrajectory();
    void createProjectile();
    void launchProjectile();
    void updateUI(int projectilesLeft, int enemiesLeft, int currentLevel);
    void loadProjectileTextures();

    sf::Texture mBackgroundTexture;
    sf::Sprite mBackgroundSprite;
    sf::Texture mGroundTexture;
    sf::Sprite mGroundSprite;
    sf::Texture mBlockTexture;
    sf::Texture mEnemyTexture;
    sf::Texture mFireballTexture;
    sf::Sprite mFireballSprite;

    std::array<sf::Texture, 5> mProjectileTextures;
    std::vector<std::unique_ptr<Projectile>> mProjectiles;

    sf::Vector2f mSlingshotPos;
    sf::RectangleShape mSlingshotBase;
    bool mIsDragging;
    sf::Vector2f mDragPosition;

    const float MAX_PULL_DISTANCE = 150.f;
    const float SCALING_FACTOR = 4.5f;
    const float PROJECTILE_RADIUS = 30.f;

    std::vector<sf::Vector2f> mTrajectoryPoints;
    const int NUM_POINTS = 30;
    const float TIME_STEP_TRAJECTORY = 0.1f;

    bool mDebugDraw;

    // Constants for positioning
    const float GROUND_HEIGHT = 100.f;
    const float SLINGSHOT_WIDTH = 20.f;
    const float SLINGSHOT_HEIGHT = 200.f;

    bool mProjectileLaunched;
    sf::Clock mProjectileLifetime;
    const float PROJECTILE_LIFETIME = 8.0f; // 8 seconds

    const int MAX_PROJECTILES = 5;
    int mProjectilesLeft;
    int mEnemiesLeft;
    int mCurrentLevel;

    void createSplitProjectiles(const Projectile* originalProjectile);
    void removeDestroyedObjects();
};

#endif