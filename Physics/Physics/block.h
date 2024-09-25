#ifndef BLOCK_H
#define BLOCK_H

#include "game_object.h"

class Block : public GameObject
{
public:

    Block(const sf::Texture& texture, float width, float height);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;

    void damage(float amount);
    bool isDestroyed() const;
    float getHealth() const { return mHealth; }
    void initPhysicsBody(PhysicsWorld& world, bool isStatic = false);

private:
    float mHealth;
    static const float MAX_HEALTH;
};

#endif