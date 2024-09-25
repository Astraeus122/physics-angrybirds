#include "level_one_scene.h"
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

LevelOneScene::LevelOneScene()
    : mIsDragging(false), mDebugDraw(false),
    mProjectileLaunched(false), mProjectilesLeft(MAX_PROJECTILES), mEnemiesLeft(0), mCurrentLevel(1) {}

void LevelOneScene::initialize()
{
    // Load textures
    if (!mBackgroundTexture.loadFromFile("dependencies/sprites/background.jpg") ||
        !mGroundTexture.loadFromFile("dependencies/sprites/ground.png") ||
        !mBlockTexture.loadFromFile("dependencies/sprites/box.png") ||
        !mEnemyTexture.loadFromFile("dependencies/sprites/spider.png") ||
        !mFireballTexture.loadFromFile("dependencies/sprites/fireball.png"))
    {
        std::cerr << "Failed to load textures!" << std::endl;
        return;
    }

    loadProjectileTextures();

    // Set up background
    mBackgroundSprite.setTexture(mBackgroundTexture);
    mBackgroundSprite.setScale(
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
        std::cerr << "Failed to load debug font!" << std::endl;
    }
}

void LevelOneScene::loadProjectileTextures()
{
    mProjectileTextures[0].loadFromFile("dependencies/sprites/standard.png");
    mProjectileTextures[1].loadFromFile("dependencies/sprites/bouncy.png");
    mProjectileTextures[2].loadFromFile("dependencies/sprites/explosive.png");
    mProjectileTextures[3].loadFromFile("dependencies/sprites/heavy.png");
    mProjectileTextures[4].loadFromFile("dependencies/sprites/split.png");
}

void LevelOneScene::createGround()
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

void LevelOneScene::render(sf::RenderWindow& window)
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

    drawDebugInfo(window);
    mGameUI.renderProjectileSelection(window);
}

void LevelOneScene::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D)
    {
        mDebugDraw = !mDebugDraw;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        mGameUI.handleProjectileSelection(sf::Mouse::getPosition(*getWindow()));
    }

    if (!mProjectiles.empty() && !mProjectileLaunched)
    {
        sf::Vector2f projectileOffset(-40.f, -40.f); // Offset to match createProjectile while being pulled back

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
            mProjectiles.back()->setPosition(mDragPosition.x, mDragPosition.y);
        }
    }
}

void LevelOneScene::update(sf::Time deltaTime)
{
    Scene::update(deltaTime);

    updateTrajectory();

    // Update projectiles
    for (auto it = mProjectiles.begin(); it != mProjectiles.end();)
    {
        (*it)->update(deltaTime);

        if ((*it)->hasExceededLifetime())
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
    }

    if (mProjectiles.empty() && mProjectilesLeft > 0)
    {
        createProjectile();
    }

    checkLevelCompletion();
    updateUI(mProjectilesLeft, mEnemiesLeft, mCurrentLevel);
    removeDestroyedObjects();
}

void LevelOneScene::createProjectile()
{
    Projectile::Type type = mGameUI.getSelectedProjectileType();

    auto projectile = std::make_unique<Projectile>(mPhysicsWorld, mFireballTexture, type, mWindow);
    projectile->setPosition(mSlingshotPos.x - 40.f, mSlingshotPos.y - 40.f);
    projectile->setKinematic(true);

    // Set the onSplit callback
    projectile->onSplit = [this](const Projectile& originalProjectile)
        {
            this->createSplitProjectiles(&originalProjectile);
        };
    
    // Set type-specific properties
    switch (type)
    {
    case Projectile::Type::Heavy:
        projectile->getPhysicsBody()->SetGravityScale(1.5f); // Increased gravity
        break;
    case Projectile::Type::Bouncy:
        projectile->getPhysicsBody()->GetFixtureList()->SetRestitution(0.8f); // High bounciness
        break;
    default:
        break;
    }

    // Increase the size of the projectile
    float scaleFactor = 5.0f;
    sf::Vector2u textureSize = mFireballTexture.getSize();
    float newWidth = textureSize.x * scaleFactor;
    float newHeight = textureSize.y * scaleFactor;
    projectile->setSize(newWidth, newHeight);

    mProjectiles.push_back(std::move(projectile));
}

void LevelOneScene::createSplitProjectiles(const Projectile* originalProjectile)
{
    if (!originalProjectile) return;

    b2Vec2 position = originalProjectile->getPhysicsBody()->GetPosition();
    b2Vec2 velocity = originalProjectile->getPhysicsBody()->GetLinearVelocity();
    float speed = velocity.Length();

    const float SPLIT_ANGLE = 15.0f * b2_pi / 180.0f; // 15 degrees in radians
    float baseAngle = atan2(velocity.y, velocity.x);

    for (int i = -1; i <= 1; i += 2) // Create two new projectiles
    {
        float angle = baseAngle + i * SPLIT_ANGLE;
        b2Vec2 newVelocity(speed * cos(angle), speed * sin(angle));

        auto newProjectile = std::make_unique<Projectile>(mPhysicsWorld, mFireballTexture, Projectile::Type::Standard, mWindow);
        newProjectile->setPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);
        newProjectile->getPhysicsBody()->SetLinearVelocity(newVelocity);
        newProjectile->setLaunched(true);

        // Prevent further splitting
        newProjectile->onSplit = nullptr;

        mProjectiles.push_back(std::move(newProjectile));
    }
}

void LevelOneScene::launchProjectile()
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

        mProjectiles.back()->setKinematic(false);
        mProjectiles.back()->launch(direction, force);
        mProjectilesLeft--;
        mProjectileLaunched = true;
        mProjectileLifetime.restart();
        updateUI(mProjectilesLeft, mEnemiesLeft, mCurrentLevel);
    }
}

void LevelOneScene::updateTrajectory()
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

void LevelOneScene::resetProjectile()
{
    if (!mProjectiles.empty())
    {
        mProjectiles.back()->setPosition(mSlingshotPos.x, mSlingshotPos.y);
        mProjectiles.back()->resetVelocity();
        mProjectiles.back()->setKinematic(true);  // Set back to kinematic
        mProjectileLaunched = false;
    }
}

void LevelOneScene::createEnemies()
{
    std::cout << "Creating enemies..." << std::endl;
    std::vector<sf::Vector2f> enemyPositions = { {1700.f, 700.f}, {1600.f, 500.f} };

    for (const auto& pos : enemyPositions)
    {
        auto enemy = std::make_unique<Enemy>(mEnemyTexture, 80.f, 80.f);
        enemy->setPosition(pos.x, pos.y);
        enemy->initPhysicsBody(mPhysicsWorld);
        mGameObjects.push_back(std::move(enemy));
    }
}

void LevelOneScene::checkLevelCompletion()
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
        // You might want to set a flag, trigger a transition, or load the next level
    }
    else if (mProjectilesLeft == 0 && (mProjectiles.empty() || mProjectileLaunched))
    {
        std::cout << "Out of projectiles! Level failed." << std::endl;
        // Handle level failure (e.g., restart level or show game over screen)
    }
}

void LevelOneScene::printAllBodies()
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

void LevelOneScene::createBlocks()
{
    std::cout << "Creating blocks..." << std::endl;
    for (int i = 0; i < 5; ++i) 
    {
        auto block = std::make_unique<Block>(mBlockTexture, 80.f, 80.f);
        float xPos = 1500.f + i * 90.f;
        float yPos = 800.f - i * 90.f;
        block->setPosition(xPos, yPos);
        block->initPhysicsBody(mPhysicsWorld); // Initialize the physics body after setting position
        mGameObjects.push_back(std::move(block));
    }
    std::cout << "Block creation complete." << std::endl;
}

void LevelOneScene::updateUI(int projectilesLeft, int enemiesLeft, int currentLevel)
{
    mGameUI.update(projectilesLeft, enemiesLeft, currentLevel);
}

void LevelOneScene::removeDestroyedObjects()
{
    auto windowSize = mWindow->getSize();
    mGameObjects.erase(
        std::remove_if(mGameObjects.begin(), mGameObjects.end(),
            [this, windowSize](const std::unique_ptr<GameObject>& obj) {
                if (obj->isMarkedForDeletion() ||
                    obj->getPosition().x < 0 || obj->getPosition().x > windowSize.x ||
                    obj->getPosition().y < 0 || obj->getPosition().y > windowSize.y)
                {
                    mPhysicsWorld.destroyBody(obj->getPhysicsBody());
                    // mPhysicsBody is set to nullptr inside destroyBody
                    return true;
                }
                return false;
            }),
        mGameObjects.end());
}

void LevelOneScene::addProjectile(std::unique_ptr<Projectile> projectile)
{
    mProjectiles.push_back(std::move(projectile));
}

void LevelOneScene::drawDebugInfo(sf::RenderWindow& window)
{
    for (const auto& obj : mGameObjects)
    {
        sf::Vector2f position = obj->getPosition();
        sf::Text debugText;
        debugText.setFont(mDebugFont);
        debugText.setCharacterSize(12);
        debugText.setFillColor(sf::Color::White);

        if (auto* enemy = dynamic_cast<Enemy*>(obj.get()))
        {
            debugText.setString("HP: " + std::to_string(enemy->getHealth()));
        }
        else if (auto* block = dynamic_cast<Block*>(obj.get()))
        {
            debugText.setString("HP: " + std::to_string(block->getHealth()));
        }

        debugText.setPosition(position.x, position.y - 20);
        window.draw(debugText);
    }

    // Draw projectile debug info
    for (const auto& projectile : mProjectiles)
    {
        sf::Vector2f position = projectile->getPosition();
        sf::Text debugText;
        debugText.setFont(mDebugFont);
        debugText.setCharacterSize(12);
        debugText.setFillColor(sf::Color::Yellow);
        debugText.setString("Type: " + std::to_string(static_cast<int>(projectile->getType())));
        debugText.setPosition(position.x, position.y - 20);
        window.draw(debugText);
    }
}

bool LevelOneScene::checkCollision(GameObject* obj1, GameObject* obj2)
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

bool LevelOneScene::isProjectileOffScreen(const std::unique_ptr<Projectile>& projectile) const
{
    sf::Vector2f position = projectile->getPosition();
    sf::Vector2u windowSize = mWindow->getSize();
    float buffer = 100.0f; // Allow some buffer off-screen

    return position.x < -buffer || position.x > windowSize.x + buffer ||
        position.y < -buffer || position.y > windowSize.y + buffer;
}
