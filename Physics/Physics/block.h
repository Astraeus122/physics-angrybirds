#ifndef BLOCK_H
#define BLOCK_H

#include "game_object.h"

class Block : public GameObject
{
public:
    Block(PhysicsWorld& world, const sf::Texture& texture, float width, float height, bool isStatic = false);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;

    void damage(float amount);
    bool isDestroyed() const;
    float getHealth() const { return mHealth; }

private:
    float mHealth;
    static const float MAX_HEALTH;
};

#endif