#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include <box2d/box2d.h>
#include <vector>
#include <memory>
#include <functional>
#include <SFML/Graphics.hpp>

class GameObject;
class Projectile;

class PhysicsWorld : public b2ContactListener
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void update(float deltaTime);
    b2Body* createBody(const b2BodyDef& bodyDef);
    void destroyBody(b2Body* body);
    void logBodyCreation(const b2BodyDef& bodyDef, const char* source) const;
    b2Joint* createJoint(const b2JointDef& jointDef);
    void destroyJoint(b2Joint* joint);
    void setGravity(float x, float y);
    void registerGameObject(std::unique_ptr<GameObject> gameObject);
    void unregisterGameObject(GameObject* gameObject);
    b2World* getWorld() const { return mWorld.get(); }

    // Collision handling
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;

    // Projectile effects
    void applyExplosionForce(const b2Vec2& center, float radius, float force);
    void applyBounceEffect(Projectile* projectile);

    void removeMarkedBodies();

    static constexpr float SCALE = 30.f;
    static constexpr float INVERSE_SCALE = 1.f / SCALE;

    void addProjectile(std::unique_ptr<Projectile> projectile);
    void setWindow(sf::RenderWindow* window) { mWindow = window; }

private:
    std::unique_ptr<b2World> mWorld;
    std::vector<std::unique_ptr<GameObject>> mGameObjects;
    static const int VELOCITY_ITERATIONS = 8;
    static const int POSITION_ITERATIONS = 3;
    sf::RenderWindow* mWindow;

    void cleanupMarkedObjects();
    void queryAABB(const b2AABB& aabb, std::function<bool(b2Fixture*)> callback);
};

#endif