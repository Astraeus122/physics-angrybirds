#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

class PhysicsWorld;

class GameObject
{
public:
    GameObject();
    virtual ~GameObject();

    const sf::Sprite& getSprite() const { return mSprite; }

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual void onCollision(GameObject* other) = 0;

    virtual void setPosition(float x, float y);
    virtual void setRotation(float angle);
    virtual sf::Vector2f getPosition() const;
    virtual float getRotation() const;

    void setTexture(const sf::Texture& texture);
    void setSize(float width, float height);

    b2Body* getPhysicsBody() const;
    void createPhysicsBody(PhysicsWorld& world, b2BodyType type);
    void markForDeletion();
    bool isMarkedForDeletion() const;

    virtual void debugDraw(sf::RenderWindow& window);
    void setPhysicsBody(b2Body* body);

protected:
    sf::Sprite mSprite;
    b2Body* mPhysicsBody;
    bool mMarkedForDeletion;
};

#endif