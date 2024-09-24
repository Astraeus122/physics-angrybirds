#include "projectile.h"
#include "physics_world.h"
#include "enemy.h"
#include "block.h"
#include <iostream>

Projectile::Projectile(PhysicsWorld& world, const sf::Texture& texture, Type type, sf::RenderWindow* window)
    : GameObject(), mType(type), mBaseDamage(0), mExplosionRadius(0), mBounceCount(0),
    mSplitAngle(0), mLaunched(false), mPhysicsWorldPtr(&world), mWindow(window),
    mLifetime(sf::seconds(8.0f))
{
    setTexture(texture);

    // Calculate scale factor
    float scaleFactor = 5.0f;
    sf::Vector2u textureSize = texture.getSize();
    float width = textureSize.x * scaleFactor;
    float height = textureSize.y * scaleFactor;

    setSize(width, height);

    // Center the origin of the sprite
    mSprite.setOrigin(textureSize.x / 2.f, textureSize.y / 2.f);

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(getPosition().x / PhysicsWorld::SCALE, getPosition().y / PhysicsWorld::SCALE);
    bodyDef.fixedRotation = true; // Prevent rotation
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(this);
    bodyDef.userData.pointer = 0;

    mPhysicsBody = world.createBody(bodyDef);

    // Create a circular shape for the fireball
    b2CircleShape circleShape;
    circleShape.m_radius = (width / 2.0f) / PhysicsWorld::SCALE; // Use half of the width as radius

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    mPhysicsBody->CreateFixture(&fixtureDef);

    switch (mType)
    {
    case Type::Standard:
        mBaseDamage = 50.0f;
        break;
    case Type::Bouncy:
        mBaseDamage = 30.0f;
        mBounceCount = MAX_BOUNCES;
        mPhysicsBody->GetFixtureList()->SetRestitution(0.8f);
        break;
    case Type::Explosive:
        mBaseDamage = 70.0f;
        mExplosionRadius = 100.0f;
        break;
    case Type::Heavy:
        mBaseDamage = 100.0f;
        mPhysicsBody->SetGravityScale(1.5f);
        break;
    case Type::Split:
        mBaseDamage = 40.0f;
        mSplitAngle = 30.0f;
        break;
    }
}

void Projectile::update(sf::Time deltaTime)
{
    GameObject::update(deltaTime);

    if (mPhysicsBody && mWindow)
    {
        b2Vec2 position = mPhysicsBody->GetPosition();
        float angle = mPhysicsBody->GetAngle();
        setPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);
        setRotation(angle * 180.f / b2_pi);

        // Check if projectile is off-screen
        sf::Vector2u windowSize = mWindow->getSize();
        if (position.x < 0 || position.x > windowSize.x / PhysicsWorld::SCALE ||
            position.y < 0 || position.y > windowSize.y / PhysicsWorld::SCALE)
        {
            markForDeletion();
        }
    }

    // Update lifetime
    mLifetime -= deltaTime;
    if (mLifetime <= sf::Time::Zero)
    {
        markForDeletion();
    }
}

void Projectile::render(sf::RenderWindow& window)
{
    window.draw(mSprite);
}

void Projectile::onCollision(GameObject* other)
{
    if (other)
    {
        // Existing collision handling code
        if (mPhysicsWorldPtr && mLaunched)
        {
            applyEffect(*mPhysicsWorldPtr, other);
        }
    }
    else
    {
        // Handle collision with static objects like ground 
    }
}


void Projectile::launch(const sf::Vector2f& direction, float force)
{
    b2Vec2 velocity(direction.x * force * PhysicsWorld::INVERSE_SCALE,
        direction.y * force * PhysicsWorld::INVERSE_SCALE);
    mPhysicsBody->SetLinearVelocity(velocity);
    mLaunched = true;
}

float Projectile::calculateDamage() const
{
    float velocityMagnitude = mPhysicsBody->GetLinearVelocity().Length();
    return mBaseDamage * velocityMagnitude * PhysicsWorld::SCALE / 100.0f;
}

void Projectile::applyEffect(PhysicsWorld& world, GameObject* other)
{
    switch (mType)
    {
    case Type::Bouncy:
        if (mBounceCount > 0)
        {
            mBounceCount--;
            b2Vec2 velocity = mPhysicsBody->GetLinearVelocity();
            mPhysicsBody->SetLinearVelocity(b2Vec2(velocity.x * BOUNCE_VELOCITY_FACTOR,
                velocity.y * BOUNCE_VELOCITY_FACTOR));
        }
        else
        {
            markForDeletion(); // Remove the projectile after maximum bounces
        }
        break;
    case Type::Explosive:
    {
        b2Vec2 position = mPhysicsBody->GetPosition();
        world.applyExplosionForce(position, mExplosionRadius * PhysicsWorld::INVERSE_SCALE, mBaseDamage);
        createExplosionEffect();
        markForDeletion(); // Ensure the projectile is destroyed after exploding
    }
    break;
    case Type::Split:
        if (onSplit)
        {
            onSplit(*this);
        }
        markForDeletion();
        break;
    case Type::Heavy:
    case Type::Standard:
        if (auto* enemy = dynamic_cast<Enemy*>(other))
        {
            float damage = calculateDamage();
            if (mType == Type::Heavy) damage *= 2.0f; // Double damage for heavy projectiles
            enemy->damage(damage);
        }
        else if (auto* block = dynamic_cast<Block*>(other))
        {
            float damage = calculateDamage();
            if (mType == Type::Heavy) damage *= 2.0f; // Double damage for heavy projectiles
            block->damage(damage);
        }
        break;
    }
}

void Projectile::createSplitProjectiles(PhysicsWorld& world)
{
    b2Vec2 position = mPhysicsBody->GetPosition();
    b2Vec2 velocity = mPhysicsBody->GetLinearVelocity();
    float speed = velocity.Length();

    const float SPLIT_ANGLE = 30.0f * b2_pi / 180.0f; // 30 degrees in radians

    for (int i = -1; i <= 1; i += 2) // Create two new projectiles
    {
        float angle = atan2(velocity.y, velocity.x) + i * SPLIT_ANGLE;
        b2Vec2 newVelocity(speed * cos(angle), speed * sin(angle));

        auto newProjectile = std::make_unique<Projectile>(world, *mSprite.getTexture(), Type::Standard);
        newProjectile->setPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);
        newProjectile->getPhysicsBody()->SetLinearVelocity(newVelocity);
        newProjectile->mLaunched = true;

        world.addProjectile(std::move(newProjectile));
    }
}

void Projectile::setKinematic(bool isKinematic)
{
    if (mPhysicsBody)
    {
        mPhysicsBody->SetType(isKinematic ? b2_kinematicBody : b2_dynamicBody);
    }
}

void Projectile::resetVelocity()
{
    if (mPhysicsBody)
    {
        mPhysicsBody->SetLinearVelocity(b2Vec2(0, 0));
        mPhysicsBody->SetAngularVelocity(0);
    }
}

const sf::Texture* Projectile::getTexture() const
{
    return mSprite.getTexture();
}

void Projectile::createExplosionEffect()
{
    // Create a circular shape for the explosion effect
    sf::CircleShape explosionShape(mExplosionRadius * PhysicsWorld::SCALE);
    explosionShape.setFillColor(sf::Color(255, 0, 0, 128)); // Semi-transparent red
    explosionShape.setPosition(getPosition().x - mExplosionRadius * PhysicsWorld::SCALE,
        getPosition().y - mExplosionRadius * PhysicsWorld::SCALE);
}   
