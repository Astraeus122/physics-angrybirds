#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "game_object.h"
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <functional>

class PhysicsWorld;

class Projectile : public GameObject
{
public:
    enum class Type
    {
        Standard,
        Bouncy,
        Explosive,
        Heavy,
        Split
    };

    Projectile(PhysicsWorld& world, const sf::Texture& texture, Type type, sf::RenderWindow* window = nullptr);
    virtual ~Projectile() = default;

    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;

    void launch(const sf::Vector2f& direction, float force);
    Type getType() const { return mType; }
    bool isLaunched() const { return mLaunched; }

    float calculateDamage() const;

    void setKinematic(bool isKinematic);
    void resetVelocity();

    const sf::Texture* getTexture() const;

    void applyEffect(PhysicsWorld& world, GameObject* other);

    void setLaunched(bool launched) { mLaunched = launched; }

    bool hasExceededLifetime() const { return mLifetime <= sf::Time::Zero; }

    std::function<void(const Projectile&)> onSplit;
    bool isEffectActive() const;

private:
    Type mType;
    float mBaseDamage;
    float mExplosionRadius; // For explosive projectiles
    int mBounceCount; // For bouncy projectiles
    float mSplitAngle; // For split projectiles
    static constexpr float BOUNCE_VELOCITY_FACTOR = 1.1f;
    static constexpr int MAX_BOUNCES = 5;

    bool mLaunched;
    sf::Time mLifetime;
    sf::RenderWindow* mWindow;

    void createSplitProjectiles(PhysicsWorld& world);
    void createExplosionEffect();
    PhysicsWorld* mPhysicsWorldPtr; // Store a pointer to PhysicsWorld

    sf::CircleShape mExplosionShape;
    bool mHasExplosionEffect = false;
    sf::Time mExplosionEffectDuration = sf::seconds(0.5f);
    sf::Time mExplosionEffectTimer = sf::Time::Zero;
    bool mHasExploded = false;
};

#endif