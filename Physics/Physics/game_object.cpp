#include "game_object.h"
#include "physics_world.h"
#include <iostream>

GameObject::GameObject() : mPhysicsBody(nullptr), mMarkedForDeletion(false) {}

GameObject::~GameObject() {}

void GameObject::markForDeletion() 
{
    mMarkedForDeletion = true;
}

bool GameObject::isMarkedForDeletion() const
{
    return mMarkedForDeletion;
}

void GameObject::setPosition(float x, float y)
{
    mSprite.setPosition(x, y);
    if (mPhysicsBody && !isMarkedForDeletion())
    {
        mPhysicsBody->SetTransform
        (
            b2Vec2(x * PhysicsWorld::INVERSE_SCALE, y * PhysicsWorld::INVERSE_SCALE),
            mPhysicsBody->GetAngle()
        );
    }
}

void GameObject::update(sf::Time deltaTime)
{
    if (mPhysicsBody)
    {
        b2Vec2 position = mPhysicsBody->GetPosition();
        float angle = mPhysicsBody->GetAngle();
        mSprite.setPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);
        mSprite.setRotation(angle * 180.f / b2_pi);
    }
}


void GameObject::setRotation(float angle) 
{
    mSprite.setRotation(angle);
    if (mPhysicsBody)
    {
        mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), angle * b2_pi / 180.f);
    }
}

sf::Vector2f GameObject::getPosition() const
{
    return mSprite.getPosition();
}

float GameObject::getRotation() const 
{
    return mSprite.getRotation();
}

void GameObject::setTexture(const sf::Texture& texture) 
{
    mSprite.setTexture(texture);
}

void GameObject::setSize(float width, float height)
{
    sf::Vector2u textureSize = mSprite.getTexture()->getSize();
    mSprite.setScale
    (
        width / static_cast<float>(textureSize.x),
        height / static_cast<float>(textureSize.y)
    );

    if (mPhysicsBody)
    {
        // Remove old fixtures
        while (mPhysicsBody->GetFixtureList())
        {
            mPhysicsBody->DestroyFixture(mPhysicsBody->GetFixtureList());
        }

        // Create new fixture with updated size
        b2PolygonShape shape;
        shape.SetAsBox((width / 2) * PhysicsWorld::INVERSE_SCALE, (height / 2) * PhysicsWorld::INVERSE_SCALE);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;
        fixtureDef.restitution = 0.1f;

        mPhysicsBody->CreateFixture(&fixtureDef);
    }
}

b2Body* GameObject::getPhysicsBody() const 
{
    return mPhysicsBody;
}

void GameObject::createPhysicsBody(PhysicsWorld& world, b2BodyType type)
{
    b2BodyDef bodyDef;
    bodyDef.type = type;
    bodyDef.position.Set(mSprite.getPosition().x * PhysicsWorld::INVERSE_SCALE,
        mSprite.getPosition().y * PhysicsWorld::INVERSE_SCALE);
    bodyDef.angle = mSprite.getRotation() * b2_pi / 180.f;
    bodyDef.fixedRotation = false;
    bodyDef.bullet = true;
    bodyDef.angularDamping = 0.8f;
    bodyDef.linearDamping = 0.1f;
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(this);

    mPhysicsBody = world.createBody(bodyDef);

    if (!mPhysicsBody)
    {
        std::cout << "Failed to create physics body!" << std::endl;
        return;
    }

    float width = mSprite.getGlobalBounds().width;
    float height = mSprite.getGlobalBounds().height;

    if (width <= 0 || height <= 0)
    {
        std::cout << "Invalid sprite dimensions: " << width << "x" << height << std::endl;
        return;
    }

    b2PolygonShape shape;
    shape.SetAsBox((width / 2) * PhysicsWorld::INVERSE_SCALE,
        (height / 2) * PhysicsWorld::INVERSE_SCALE);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.1f;

    b2Fixture* fixture = mPhysicsBody->CreateFixture(&fixtureDef);

    if (!fixture)
    {
        std::cout << "Failed to create fixture!" << std::endl;
    }
}

void GameObject::debugDraw(sf::RenderWindow& window)
{
    if (mPhysicsBody)
    {
        for (b2Fixture* fixture = mPhysicsBody->GetFixtureList(); fixture; fixture = fixture->GetNext())
        {
            switch (fixture->GetType())
            {
            case b2Shape::e_circle:
            {
                b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();
                sf::CircleShape shape(circle->m_radius * PhysicsWorld::SCALE);
                shape.setOrigin(circle->m_radius * PhysicsWorld::SCALE, circle->m_radius * PhysicsWorld::SCALE);
                shape.setPosition(mPhysicsBody->GetPosition().x * PhysicsWorld::SCALE,
                    mPhysicsBody->GetPosition().y * PhysicsWorld::SCALE);
                shape.setRotation(mPhysicsBody->GetAngle() * 180 / b2_pi);
                shape.setFillColor(sf::Color::Transparent);
                shape.setOutlineColor(sf::Color::Green);
                shape.setOutlineThickness(1.0f);
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
                    b2Vec2 point = mPhysicsBody->GetWorldPoint(poly->m_vertices[i]);
                    shape.setPoint(i, sf::Vector2f(point.x * PhysicsWorld::SCALE, point.y * PhysicsWorld::SCALE));
                }
                shape.setFillColor(sf::Color::Transparent);
                shape.setOutlineColor(sf::Color::Green);
                shape.setOutlineThickness(1.0f);
                window.draw(shape);
                break;
            }
            }
        }
    }
}

void GameObject::setPhysicsBody(b2Body* body)
{
    mPhysicsBody = body;
}
