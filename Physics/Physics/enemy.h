#ifndef ENEMY_H
#define ENEMY_H

#include "game_object.h"

class Enemy : public GameObject
{
public:

    Enemy(const sf::Texture& texture, float width, float height);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;

    void damage(float amount);
    bool isDefeated() const;

    float getHealth() const { return mHealth; }
    void initPhysicsBody(PhysicsWorld& world);

private:
    float mHealth;
    static const float MAX_HEALTH;
};

#endif