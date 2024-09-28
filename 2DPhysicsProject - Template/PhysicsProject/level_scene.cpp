#include "level_scene.h"
#include "block.h"
#include "enemy.h"
#include "projectile.h"
#include <iostream>
#include <algorithm>
#include <cmath>

const float SCALE = 30.f;

inline float toBox2D(float pixel)
{
    return pixel / SCALE;
}

inline float toPixel(float meter)
{
    return meter * SCALE;
}

LevelScene::LevelScene() : mCurrentLevel(1), mIsDragging(false), mDebugDraw(false),
mProjectileLaunched(false), mProjectilesLeft(MAX_PROJECTILES), mEnemiesLeft(0),
mLevelCompleted(false), mLevelFailed(false), mFinalProjectileLaunched(false) {}

void LevelScene::initialize()
{
    // Load textures
    if (!mBackgroundTexture.loadFromFile("dependencies/sprites/background.jpg") ||
        !mGroundTexture.loadFromFile("dependencies/sprites/ground.png") ||
        !mBlockTexture.loadFromFile("dependencies/sprites/box.png") ||
        !mEnemyTexture.loadFromFile("dependencies/sprites/spider.png") ||
        !mFireballTexture.loadFromFile("dependencies/sprites/fireball.png"))
    {
        std::cout << "Failed to load textures" << std::endl;
        return;
    }

    loadProjectileTextures();

    // Set up background
    mBackgroundSprite.setTexture(mBackgroundTexture);
    mBackgroundSprite.setScale
    (
        getWindow()->getSize().x / static_cast<float>(mBackgroundTexture.getSize().x),
        getWindow()->getSize().y / static_cast<float>(mBackgroundTexture.getSize().y)
    );

    // Set up ground
    mGroundSprite.setTexture(mGroundTexture);
    mGroundSprite.setTextureRect(sf::IntRect(0, 0, getWindow()->getSize().x, GROUND_HEIGHT));
    mGroundSprite.setPosition(0, getWindow()->getSize().y - GROUND_HEIGHT);

    // Set up fireball sprite
    mFireballSprite.setTexture(mFireballTexture);
    mFireballSprite.setOrigin(mFireballTexture.getSize().x / 1.0f, mFireballTexture.getSize().y / 2.f);
    mFireballSprite.setPosition(mSlingshotPos);

    // Set up slingshot
    mSlingshotPos = sf::Vector2f(200.f, getWindow()->getSize().y - GROUND_HEIGHT - SLINGSHOT_HEIGHT);
    mSlingshotBase.setSize(sf::Vector2f(SLINGSHOT_WIDTH, SLINGSHOT_HEIGHT));
    mSlingshotBase.setFillColor(sf::Color::Yellow);
    mSlingshotBase.setPosition(mSlingshotPos.x - SLINGSHOT_WIDTH / 2, mSlingshotPos.y);

    createGround();
    createBlocks();
    createEnemies();
    createProjectile();

    updateUI(mProjectilesLeft, mEnemiesLeft, mCurrentLevel);

    if (!mDebugFont.loadFromFile("dependencies/font.ttf"))
    {
        std::cout << "Failed to load debug font" << std::endl;
    }
}

void LevelScene::loadProjectileTextures()
{
    mProjectileTextures[0].loadFromFile("dependencies/sprites/standard.png");
    mProjectileTextures[1].loadFromFile("dependencies/sprites/bouncy.png");
    mProjectileTextures[2].loadFromFile("dependencies/sprites/explosive.png");
    mProjectileTextures[3].loadFromFile("dependencies/sprites/heavy.png");
    mProjectileTextures[4].loadFromFile("dependencies/sprites/split.png");
}

void LevelScene::createGround()
{
    std::cout << "Creating ground..." << std::endl;
    float groundWidth = getWindow()->getSize().x;
    float groundY = getWindow()->getSize().y - GROUND_HEIGHT;

    b2BodyDef groundBodyDef;
    groundBodyDef.type = b2_staticBody;
    groundBodyDef.position.Set(groundWidth / 2 * PhysicsWorld::INVERSE_SCALE,
        (groundY + GROUND_HEIGHT / 2) * PhysicsWorld::INVERSE_SCALE);
    groundBodyDef.userData.pointer = 0; // Initialize userData.pointer to zero

    b2Body* groundBody = mPhysicsWorld.createBody(groundBodyDef);

    b2PolygonShape groundShape;
    groundShape.SetAsBox((groundWidth / 2) * PhysicsWorld::INVERSE_SCALE, (GROUND_HEIGHT / 2) * PhysicsWorld::INVERSE_SCALE);

    b2FixtureDef groundFixtureDef;
    groundFixtureDef.shape = &groundShape;
    groundFixtureDef.friction = 0.3f;
    groundFixtureDef.restitution = 0.1f;
    groundFixtureDef.density = 0.0f;

    groundBody->CreateFixture(&groundFixtureDef);

    mGroundShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(groundWidth, GROUND_HEIGHT));
    mGroundShape->setPosition(0, groundY);
    mGroundShape->setTexture(&mGroundTexture);

    std::cout << "Ground creation complete." << std::endl;
}

void LevelScene::render(sf::RenderWindow& window)
{
    window.draw(mBackgroundSprite);

    if (mGroundShape)
    {
        window.draw(*mGroundShape);
    }

    Scene::render(window);

    // Draw slingshot
    window.draw(mSlingshotBase);

    // Draw trajectory
    if (mIsDragging && !mTrajectoryPoints.empty())
    {
        sf::VertexArray trajectory(sf::LineStrip, mTrajectoryPoints.size());
        for (size_t i = 0; i < mTrajectoryPoints.size(); ++i)
        {
            trajectory[i].position = mTrajectoryPoints[i];
            trajectory[i].color = sf::Color::Red;
        }
        window.draw(trajectory);
    }

    // Draw projectiles
    for (const auto& projectile : mProjectiles)
    {
        projectile->render(window);
    }

    if (mDebugDraw)
    {
        b2Body* body = mPhysicsWorld.getWorld()->GetBodyList();
        while (body != nullptr)
        {
            b2Vec2 pos = body->GetPosition();
            float angle = body->GetAngle();

            for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
            {
                switch (fixture->GetType())
                {
                case b2Shape::e_circle:
                {
                    b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();
                    sf::CircleShape shape(toPixel(circle->m_radius));
                    shape.setPosition(toPixel(pos.x - circle->m_radius), toPixel(pos.y - circle->m_radius));
                    shape.setRotation(angle * 180 / b2_pi);
                    shape.setFillColor(sf::Color::Transparent);
                    shape.setOutlineColor(sf::Color::Green);
                    shape.setOutlineThickness(1);
                    window.draw(shape);
                    break;
                }
                case b2Shape::e_polygon:
                {
                    b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
                    sf::ConvexShape shape;
                    shape.setPointCount(poly->m_count);
                    for (int i = 0; i < poly->m_count; ++i)
                    {
                        b2Vec2 vertex = b2Mul(body->GetTransform(), poly->m_vertices[i]);
                        shape.setPoint(i, sf::Vector2f(toPixel(vertex.x), toPixel(vertex.y)));
                    }
                    shape.setFillColor(sf::Color::Transparent);
                    shape.setOutlineColor(sf::Color::Green);
                    shape.setOutlineThickness(1);
                    window.draw(shape);
                    break;
                }
                }
            }
            body = body->GetNext();
        }
    }

    mGameUI.renderProjectileSelection(window);
}

void LevelScene::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D)
    {
        mDebugDraw = !mDebugDraw;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        mGameUI.handleMouseHover(sf::Mouse::getPosition(*getWindow()));
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        mGameUI.handleMouseClick(sf::Mouse::getPosition(*getWindow()));
        updateProjectileType();
    }

    if (!mProjectiles.empty() && !mProjectileLaunched)
    {
        sf::Vector2f projectileOffset(-40.f, -40.f);

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mousePos = getWindow()->mapPixelToCoords(sf::Mouse::getPosition(*getWindow()));
            if (mProjectiles.back()->getSprite().getGlobalBounds().contains(mousePos))
            {
                mIsDragging = true;
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && mIsDragging)
        {
            mIsDragging = false;
            launchProjectile();
        }

        if (event.type == sf::Event::MouseMoved && mIsDragging)
        {
            sf::Vector2f mousePos = getWindow()->mapPixelToCoords(sf::Mouse::getPosition(*getWindow()));
            sf::Vector2f pull = (mSlingshotPos + projectileOffset) - mousePos;
            float pullLength = std::sqrt(pull.x * pull.x + pull.y * pull.y);
            if (pullLength > MAX_PULL_DISTANCE)
            {
                pull = (pull / pullLength) * MAX_PULL_DISTANCE;
            }
            mDragPosition = (mSlingshotPos + projectileOffset) - pull;

            mProjectiles.back()->updatePosition(mDragPosition.x, mDragPosition.y);
        }
    }
}

void LevelScene::update(sf::Time deltaTime)
{
    Scene::update(deltaTime);

    // Process ending split projectile
    for (const auto& action : mPendingActions)
    {
        action();
    }
    mPendingActions.clear();

    updateTrajectory();

    // Update projectiles
    for (auto it = mProjectiles.begin(); it != mProjectiles.end();)
    {
        (*it)->update(deltaTime);

        if ((*it)->hasExceededLifetime() || isProjectileOffScreen(*it))
        {
            mPhysicsWorld.destroyBody((*it)->getPhysicsBody());
            it = mProjectiles.erase(it);
        }
        else if ((*it)->isMarkedForDeletion())
        {
            // Check if the projectile still has active effects
            if (!(*it)->isEffectActive())
            {
                mPhysicsWorld.destroyBody((*it)->getPhysicsBody());
                it = mProjectiles.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }

    // Reset mProjectileLaunched if no projectiles are active
    if (mProjectiles.empty())
    {
        mProjectileLaunched = false;
        if (mProjectilesLeft > 0)
        {
            createProjectile(); // Create a new projectile when all are gone and we have projectiles left
        }
        else if (mFinalProjectileLaunched)
        {
            // The final projectile has completed its trajectory
            checkLevelCompletion();
        }
    }

    checkLevelCompletion();
    updateUI(mProjectilesLeft, mEnemiesLeft, mCurrentLevel);
    removeDestroyedObjects();
}

void LevelScene::createProjectile()
{
    Projectile::Type type = mGameUI.getSelectedProjectileType();
    sf::Vector2f position(mSlingshotPos.x - 40.f, mSlingshotPos.y - 40.f);

    auto projectile = std::make_unique<Projectile>(mPhysicsWorld, mFireballTexture, type, mWindow, position);

    if (!mGameUI.isProjectileAvailable(type))
    {
        type = Projectile::Type::Standard;
        mGameUI.setSelectedProjectileType(type);
    }

    // You cannot create or effect the physics world during the physics simulation step, specifically here im trying to do it during a collision callback
    // To fix this issue I have to schedule the creaiton of the split projectiles, and do them after the physics step
    projectile->onSplit = [this](const Projectile& originalProjectile)
        {
            b2Vec2 position = originalProjectile.getPhysicsBody()->GetPosition();
            b2Vec2 velocity = originalProjectile.getPhysicsBody()->GetLinearVelocity();
            float speed = velocity.Length();

            const float SPLIT_ANGLE = 15.0f * b2_pi / 180.0f; // 15 degrees in radians
            float baseAngle = atan2(velocity.y, velocity.x);

            // Schedule the creation of split projectiles
            mPendingActions.push_back([this, position, speed, baseAngle, SPLIT_ANGLE]()
                {
                    for (int i = -1; i <= 1; i += 2) // Create two new projectiles
                    {
                        float angle = baseAngle + i * SPLIT_ANGLE;
                        b2Vec2 newVelocity(speed * cos(angle), speed * sin(angle));

                        sf::Vector2f newPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);

                        auto newProjectile = std::make_unique<Projectile>
                            (
                                mPhysicsWorld,
                                mFireballTexture,
                                Projectile::Type::Standard,
                                mWindow,
                                newPosition
                            );

                        newProjectile->getPhysicsBody()->SetLinearVelocity(newVelocity);
                        newProjectile->setLaunched(true);

                        // Prevent further splitting
                        newProjectile->onSplit = nullptr;

                        mProjectiles.push_back(std::move(newProjectile));
                    }
                });
        };

    float scaleFactor = 5.0f;
    sf::Vector2u textureSize = mFireballTexture.getSize();
    float newWidth = textureSize.x * scaleFactor;
    float newHeight = textureSize.y * scaleFactor;
    projectile->setSize(newWidth, newHeight);

    mProjectiles.push_back(std::move(projectile));
}

void LevelScene::launchProjectile()
{
    if (!mProjectiles.empty() && !mProjectileLaunched)
    {
        sf::Vector2f pull = mSlingshotPos - mDragPosition;
        float pullLength = std::sqrt(pull.x * pull.x + pull.y * pull.y);
        if (pullLength > MAX_PULL_DISTANCE)
        {
            pull = (pull / pullLength) * MAX_PULL_DISTANCE;
            pullLength = MAX_PULL_DISTANCE;
        }

        sf::Vector2f direction = pull / pullLength;
        float force = pullLength * SCALING_FACTOR;
        force = force * 1.08f;

        mProjectiles.back()->launch(direction, force);
        mProjectilesLeft--;
        mProjectileLaunched = true;
        mProjectileLifetime.restart();

        if (mProjectilesLeft == 0)
        {
            mFinalProjectileLaunched = true;
        }

        Projectile::Type launchedType = mProjectiles.back()->getProjectileType();
        mGameUI.updateProjectileAvailability(launchedType, false);

        updateUI(mProjectilesLeft, mEnemiesLeft, mCurrentLevel);
    }
}

void LevelScene::updateTrajectory()
{
    if (mIsDragging && !mProjectiles.empty())
    {
        mTrajectoryPoints.clear();

        sf::Vector2f pull = mSlingshotPos - mDragPosition;
        float pullLength = std::sqrt(pull.x * pull.x + pull.y * pull.y);
        if (pullLength > MAX_PULL_DISTANCE)
        {
            pull = (pull / pullLength) * MAX_PULL_DISTANCE;
            pullLength = MAX_PULL_DISTANCE;
        }

        sf::Vector2f direction = pull / pullLength;
        float force = pullLength * SCALING_FACTOR;

        // Convert to Box2D coordinates and scaling
        b2Vec2 initialVelocity(direction.x * force * PhysicsWorld::INVERSE_SCALE,
            direction.y * force * PhysicsWorld::INVERSE_SCALE);

        b2Vec2 gravity = mPhysicsWorld.getWorld()->GetGravity();
        b2Vec2 position(toBox2D(mSlingshotPos.x), toBox2D(mSlingshotPos.y));

        for (int i = 1; i <= NUM_POINTS; ++i)
        {
            float t = i * TIME_STEP_TRAJECTORY;

            // Use Box2D physics equations
            float x = position.x + initialVelocity.x * t;
            float y = position.y + initialVelocity.y * t + 0.5f * gravity.y * t * t;

            mTrajectoryPoints.emplace_back(toPixel(x), toPixel(y));
        }
    }
}

void LevelScene::resetProjectile()
{
    if (!mProjectiles.empty())
    {
        mProjectiles.back()->setPosition(mSlingshotPos.x, mSlingshotPos.y);
        mProjectiles.back()->resetVelocity();
        mProjectiles.back()->setKinematic(true);  // Set back to kinematic
        mProjectileLaunched = false;
    }
}

void LevelScene::checkLevelCompletion()
{
    mEnemiesLeft = 0;
    for (const auto& obj : mGameObjects)
    {
        if (dynamic_cast<Enemy*>(obj.get()))
        {
            mEnemiesLeft++;
        }
    }

    updateUI(mProjectilesLeft, mEnemiesLeft, mCurrentLevel);

    if (mEnemiesLeft == 0)
    {
        std::cout << "Level completed!" << std::endl;
        mLevelCompleted = true;
    }
    else if (mProjectilesLeft == 0 && mFinalProjectileLaunched && mProjectiles.empty())
    {
        std::cout << "Out of projectiles! Level failed." << std::endl;
        mLevelFailed = true;
    }
}

void LevelScene::printAllBodies()
{
    std::cout << "Printing all bodies in the world:" << std::endl;
    b2Body* body = mPhysicsWorld.getWorld()->GetBodyList();
    int bodyCount = 0;
    while (body != nullptr)
    {
        b2Vec2 position = body->GetPosition();
        float angle = body->GetAngle();
        std::cout << "Body " << bodyCount << ": Position(" << position.x << ", " << position.y
            << "), Angle: " << angle << ", Type: ";

        switch (body->GetType())
        {
        case b2_staticBody: std::cout << "Static"; break;
        case b2_kinematicBody: std::cout << "Kinematic"; break;
        case b2_dynamicBody: std::cout << "Dynamic"; break;
        }
        std::cout << std::endl;

        bodyCount++;
        body = body->GetNext();
    }
    std::cout << "Total bodies: " << bodyCount << std::endl;
}

void LevelScene::createBlocks()
{
    // Clear existing blocks
    mGameObjects.erase(
        std::remove_if(mGameObjects.begin(), mGameObjects.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                return dynamic_cast<Block*>(obj.get()) != nullptr;
            }),
        mGameObjects.end());

    const float BLOCK_SIZE = 80.f;
    const float GROUND_Y = mWindow->getSize().y - GROUND_HEIGHT;

    // Variables for block creation
    std::unique_ptr<Block> block;
    float xPos, yPos;

    // Create blocks based on current level
    switch (mCurrentLevel)
    {
    case 1:
        // Level 1: Spread out structures, closer to slingshot
    {
        // Left structure: Small tower
        for (int i = 0; i < 4; ++i) 
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 800.f;
            yPos = GROUND_Y - (i + 1) * BLOCK_SIZE;
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }

        // Middle structure: Arch
        // Left pillar
        for (int i = 0; i < 3; ++i)
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 1100.f;
            yPos = GROUND_Y - (i + 1) * BLOCK_SIZE;
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }
        // Right pillar
        for (int i = 0; i < 3; ++i) 
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 1260.f;
            yPos = GROUND_Y - (i + 1) * BLOCK_SIZE;
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }
        // Arch top
        block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE * 2, BLOCK_SIZE * 0.5f);
        xPos = 1180.f;
        yPos = GROUND_Y - 3.5f * BLOCK_SIZE;
        block->setPosition(xPos, yPos);
        block->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(block));

        // Right structure: Diagonal wall
        for (int i = 0; i < 3; ++i)
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 1500.f + i * 0.5f;
            yPos = GROUND_Y - (i + 1) * BLOCK_SIZE;
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }
    }
    break;
    case 2:
        // Level 2: Balanced seesaw structure
    {
        // Central pivot
        block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE * 3);
        xPos = 1600.f;
        yPos = GROUND_Y - BLOCK_SIZE * 1.5f;
        block->setPosition(xPos, yPos);
        block->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(block));

        // Seesaw plank
        block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE * 7, BLOCK_SIZE * 0.5f);
        xPos = 1600.f;
        yPos = GROUND_Y - BLOCK_SIZE * 3.25f;
        block->setPosition(xPos, yPos);
        block->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(block));

        // Weight on left side
        block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE * 1.5f, BLOCK_SIZE * 1.5f);
        xPos = 1360.f;
        yPos = GROUND_Y - BLOCK_SIZE * 3.75f;
        block->setPosition(xPos, yPos);
        block->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(block));

        // Stack on right side
        for (int i = 0; i < 3; ++i)
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 1840.f;
            yPos = GROUND_Y - BLOCK_SIZE * (3.75f + i);
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }
    }
    break;
    case 3:
        // Level 3: Stable multi-platform structure
    {
        // Bottom platform
        for (int i = 0; i < 7; ++i) 
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 1400.f + i * BLOCK_SIZE;
            yPos = GROUND_Y - BLOCK_SIZE;
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }

        // Left tower
        for (int i = 0; i < 4; ++i)
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 1400.f;
            yPos = GROUND_Y - (i + 2) * BLOCK_SIZE;
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }

        // Middle platform
        block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE * 3, BLOCK_SIZE * 0.5f);
        xPos = 1600.f;
        yPos = GROUND_Y - BLOCK_SIZE * 3.5f;
        block->setPosition(xPos, yPos);
        block->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(block));

        // Right tower
        for (int i = 0; i < 5; ++i) 
        {
            block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE, BLOCK_SIZE);
            xPos = 1880.f;
            yPos = GROUND_Y - (i + 2) * BLOCK_SIZE;
            block->setPosition(xPos, yPos);
            block->initPhysicsBody(mPhysicsWorld);
            mGameObjects.push_back(std::move(block));
        }

        // Top platform
        block = std::make_unique<Block>(mBlockTexture, BLOCK_SIZE * 2, BLOCK_SIZE * 0.5f);
        xPos = 1800.f;
        yPos = GROUND_Y - BLOCK_SIZE * 5.5f;
        block->setPosition(xPos, yPos);
        block->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(block));
    }
    break;

    default:
        std::cout << "Invalid level number: " << mCurrentLevel << std::endl;
        break;
    }
}

void LevelScene::createEnemies()
{
    // Clear existing enemies
    mGameObjects.erase(
        std::remove_if(mGameObjects.begin(), mGameObjects.end(),
            [](const std::unique_ptr<GameObject>& obj)
            {
                return dynamic_cast<Enemy*>(obj.get()) != nullptr;
            }),
        mGameObjects.end());

    const float ENEMY_SIZE = 80.f;
    const float GROUND_Y = mWindow->getSize().y - GROUND_HEIGHT;

    // Create enemies based on current level
    switch (mCurrentLevel)
    {
    case 1:
        // Level 1: Enemies on spread out structures, closer to slingshot
    {
        auto enemy1 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy1->setPosition(800.f, GROUND_Y - 5 * ENEMY_SIZE);
        enemy1->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy1));

        auto enemy2 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy2->setPosition(1180.f, GROUND_Y - 4.5f * ENEMY_SIZE);
        enemy2->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy2));

        auto enemy3 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy3->setPosition(1540.f, GROUND_Y - 4 * ENEMY_SIZE);
        enemy3->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy3));
    }
    break;
    case 2:
        // Level 2: Enemies on the seesaw structure
    {
        auto enemy1 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy1->setPosition(1360.f, GROUND_Y - 5.25f * ENEMY_SIZE);
        enemy1->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy1));

        auto enemy2 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy2->setPosition(1840.f, GROUND_Y - 6.75f * ENEMY_SIZE);
        enemy2->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy2));

        auto enemy3 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy3->setPosition(1600.f, GROUND_Y - 4.25f * ENEMY_SIZE);
        enemy3->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy3));
    }
    break;
    case 3:
        // Level 3: Enemies on various platforms of the structure
    {
        auto enemy1 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy1->setPosition(1400.f, GROUND_Y - 6 * ENEMY_SIZE);
        enemy1->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy1));

        auto enemy2 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy2->setPosition(1600.f, GROUND_Y - 4.5f * ENEMY_SIZE);
        enemy2->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy2));

        auto enemy3 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy3->setPosition(1880.f, GROUND_Y - 7 * ENEMY_SIZE);
        enemy3->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy3));

        auto enemy4 = std::make_unique<Enemy>(mEnemyTexture, ENEMY_SIZE, ENEMY_SIZE);
        enemy4->setPosition(1800.f, GROUND_Y - 6.5f * ENEMY_SIZE);
        enemy4->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy4));
    }
    break;

    default:
        std::cout << "Invalid level number: " << mCurrentLevel << std::endl;
        break;
    }

    mEnemiesLeft = std::count_if(mGameObjects.begin(), mGameObjects.end(),
        [](const std::unique_ptr<GameObject>& obj) {
            return dynamic_cast<Enemy*>(obj.get()) != nullptr;
        });
}

void LevelScene::updateUI(int projectilesLeft, int enemiesLeft, int currentLevel)
{
    mGameUI.update(projectilesLeft, enemiesLeft, currentLevel);
}

void LevelScene::removeDestroyedObjects()
{
    auto windowSize = mWindow->getSize();
    mGameObjects.erase(
        std::remove_if(mGameObjects.begin(), mGameObjects.end(),
            [this, windowSize](const std::unique_ptr<GameObject>& obj)
            {
                if (obj->isMarkedForDeletion() ||
                    obj->getPosition().x < 0 || obj->getPosition().x > windowSize.x ||
                    obj->getPosition().y < 0 || obj->getPosition().y > windowSize.y)
                {
                    mPhysicsWorld.destroyBody(obj->getPhysicsBody());
                    return true;
                }
                return false;
            }),
        mGameObjects.end());
}

void LevelScene::addProjectile(std::unique_ptr<Projectile> projectile)
{
    mProjectiles.push_back(std::move(projectile));
}

bool LevelScene::checkCollision(GameObject* obj1, GameObject* obj2)
{
    b2Body* body1 = obj1->getPhysicsBody();
    b2Body* body2 = obj2->getPhysicsBody();

    for (b2ContactEdge* ce = body1->GetContactList(); ce; ce = ce->next)
    {
        if (ce->other == body2 && ce->contact->IsTouching())
        {
            return true;
        }
    }

    return false;
}

bool LevelScene::isProjectileOffScreen(const std::unique_ptr<Projectile>& projectile) const
{
    sf::Vector2f position = projectile->getPosition();
    sf::Vector2u windowSize = mWindow->getSize();
    float buffer = 100.0f; // Allow some buffer off-screen

    return position.x < -buffer || position.x > windowSize.x + buffer ||
        position.y < -buffer || position.y > windowSize.y + buffer;
}

void LevelScene::updateProjectileType()
{
    if (!mProjectiles.empty() && !mProjectileLaunched)
    {
        // Destroy the physics body of the current projectile
        mPhysicsWorld.destroyBody(mProjectiles.back()->getPhysicsBody());
        // Remove the current projectile
        mProjectiles.pop_back();

        // Create a new projectile
        createProjectile();
    }
}

void LevelScene::setLevel(int level)
{
    mCurrentLevel = level;
    mLevelCompleted = false;
    mLevelFailed = false;
    mProjectilesLeft = MAX_PROJECTILES;
    mEnemiesLeft = 0;
    mFinalProjectileLaunched = false;

    // Clear existing objects
    mGameObjects.clear();
    mProjectiles.clear();

    // Recreate level elements
    createGround();
    createBlocks();
    createEnemies();

    // Reset projectile availability
    resetProjectileAvailability();

    updateUI(mProjectilesLeft, mEnemiesLeft, mCurrentLevel);
}

void LevelScene::resetProjectileAvailability()
{
    for (int i = 0; i < 5; ++i)
    {
        mGameUI.updateProjectileAvailability(static_cast<Projectile::Type>(i), true);
    }
    mGameUI.setSelectedProjectileType(Projectile::Type::Standard);
}

void LevelScene::clearLevel()
{
    // Clear all game objects
    mGameObjects.clear();

    // Clear all projectiles
    mProjectiles.clear();

    // Destroy all bodies in the physics world
    b2Body* body = mPhysicsWorld.getWorld()->GetBodyList();
    while (body)
    {
        b2Body* nextBody = body->GetNext();
        mPhysicsWorld.getWorld()->DestroyBody(body);
        body = nextBody;
    }
}