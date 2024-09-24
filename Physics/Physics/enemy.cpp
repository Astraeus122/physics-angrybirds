#include "Enemy.h"
#include "physics_world.h"
#include "projectile.h"

const float Enemy::MAX_HEALTH = 100.f;

Enemy::Enemy(PhysicsWorld& world, const sf::Texture& texture, float width, float height)
    : mHealth(MAX_HEALTH)
{
    setTexture(texture);
    setSize(width, height);

    // Calculate scale factor
    float scaleX = width / texture.getSize().x;
    float scaleY = height / texture.getSize().y;
    mSprite.setScale(scaleX, scaleY);

    // Center the origin of the sprite
    mSprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(getPosition().x / PhysicsWorld::SCALE, getPosition().y / PhysicsWorld::SCALE);
    bodyDef.fixedRotation = true; // Prevent rotation
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(this);
    bodyDef.userData.pointer = 0;

    mPhysicsBody = world.createBody(bodyDef);

    b2PolygonShape boxShape;
    boxShape.SetAsBox(
        (width / 2.0f) / PhysicsWorld::SCALE,
        (height / 2.0f) / PhysicsWorld::SCALE
    );

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = 0.9f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.15f;

    mPhysicsBody->CreateFixture(&fixtureDef);
}

void Enemy::update(sf::Time deltaTime)
{
    if (mPhysicsBody)
    {
        b2Vec2 position = mPhysicsBody->GetPosition();
        setPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);
        // We don't update rotation because fixedRotation is true
    }
}

void Enemy::render(sf::RenderWindow& window)
{
    window.draw(mSprite);
}

void Enemy::onCollision(GameObject* other)
{
    Projectile* projectile = dynamic_cast<Projectile*>(other);
    if (projectile)
    {
        float damageAmount = projectile->calculateDamage();
        this->damage(damageAmount); 
    }
}

void Enemy::damage(float amount)
{
    mHealth -= amount;
    if (mHealth <= 0)
    {
        mHealth = 0;
        markForDeletion();
    }
}

bool Enemy::isDefeated() const
{
    return mHealth <= 0;
}