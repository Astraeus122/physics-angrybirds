#include "block.h"
#include "physics_world.h"
#include "projectile.h"
#include <iostream>

const float Block::MAX_HEALTH = 100.f;

Block::Block(const sf::Texture& texture, float width, float height)
    : mHealth(MAX_HEALTH)
{
    setTexture(texture);
    setSize(width, height);

    // Calculate scale factor
    float scaleX = width / texture.getSize().x;
    float scaleY = height / texture.getSize().y;
    mSprite.setScale(scaleX, scaleY);

    // Set the sprite's origin to its center
    mSprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);
}

void Block::update(sf::Time deltaTime) 
{
    if (mPhysicsBody)
    {
        b2Vec2 position = mPhysicsBody->GetPosition();
        float angle = mPhysicsBody->GetAngle();
        setPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);
        setRotation(angle * 180.f / b2_pi);
    }
}


void Block::render(sf::RenderWindow& window)
{
    window.draw(mSprite);
}

void Block::onCollision(GameObject* other)
{
    std::cout << "Block::onCollision called with other at address: " << static_cast<void*>(other) << std::endl;
    if (other == nullptr)
    {
        std::cout << "Block::onCollision received null other object" << std::endl;
        return;
    }
    try
    {
        const char* otherType = other->getType();
        std::cout << "Type of other object: " << (otherType ? otherType : "Unknown") << std::endl;

        if (otherType && strcmp(otherType, "Projectile") == 0)
        {
            Projectile* projectile = dynamic_cast<Projectile*>(other);
            if (projectile)
            {
                std::cout << "Calculating damage..." << std::endl;
                float damageAmount = projectile->calculateDamage();
                std::cout << "Damage amount calculated: " << damageAmount << std::endl;
                this->damage(damageAmount);
            }
            else
            {
                std::cout << "Failed to cast other object to Projectile" << std::endl;
            }
        }
        else
        {
            std::cout << "Other object is not a Projectile" << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception in Block::onCollision: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown exception in Block::onCollision" << std::endl;
    }
}

void Block::damage(float amount)
{
    mHealth -= amount;
    if (mHealth <= 0)
    {
        mHealth = 0;
        markForDeletion();
    }
}

bool Block::isDestroyed() const
{
    return mHealth <= 0;
}

void Block::initPhysicsBody(PhysicsWorld& world, bool isStatic)
{
    b2BodyDef bodyDef;
    bodyDef.type = isStatic ? b2_staticBody : b2_dynamicBody;
    sf::Vector2f pos = getPosition();
    bodyDef.position.Set(pos.x / PhysicsWorld::SCALE, pos.y / PhysicsWorld::SCALE);
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(this);

    mPhysicsBody = world.createBody(bodyDef);

    b2PolygonShape boxShape;
    float width = mSprite.getGlobalBounds().width;
    float height = mSprite.getGlobalBounds().height;
    boxShape.SetAsBox((width / 2) / PhysicsWorld::SCALE, (height / 2) / PhysicsWorld::SCALE);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = isStatic ? 0.0f : 0.8f;
    fixtureDef.friction = 0.5f;
    fixtureDef.restitution = 0.2f;

    mPhysicsBody->CreateFixture(&fixtureDef);
}
