#include "physics_world.h"
#include "game_object.h"
#include "projectile.h"
#include "enemy.h"
#include "block.h"
#include <iostream>
#include <cmath>

PhysicsWorld::PhysicsWorld()
    : mWorld(std::make_unique<b2World>(b2Vec2(0, 9.81f))), mWindow(nullptr)
{
    mWorld->SetContactListener(this);
    mWorld->SetAutoClearForces(false);
    mWorld->SetContinuousPhysics(true);
    mWorld->SetSubStepping(true);
}

PhysicsWorld::~PhysicsWorld() = default;

void PhysicsWorld::update(float deltaTime)
{
    mWorld->Step(deltaTime, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    mWorld->ClearForces();
    removeMarkedBodies();
    cleanupMarkedObjects();
}

b2Body* PhysicsWorld::createBody(const b2BodyDef& bodyDef) {
    logBodyCreation(bodyDef, "Unknown");
    return mWorld->CreateBody(&bodyDef);
}

void PhysicsWorld::destroyBody(b2Body* body)
{
    if (body)
    {
        // Retrieve the GameObject associated with this body
        GameObject* gameObject = reinterpret_cast<GameObject*>(body->GetUserData().pointer);
        if (gameObject)
        {
            gameObject->setPhysicsBody(nullptr); // Set mPhysicsBody to nullptr
        }
        mWorld->DestroyBody(body);
    }
}

void PhysicsWorld::setGravity(float x, float y)
{
    mWorld->SetGravity(b2Vec2(x, y));
}

void PhysicsWorld::registerGameObject(std::unique_ptr<GameObject> gameObject)
{
    if (gameObject)
    {
        mGameObjects.push_back(std::move(gameObject));
    }
    else {
        std::cerr << "Attempted to register null game object" << std::endl;
    }
}

void PhysicsWorld::unregisterGameObject(GameObject* gameObject)
{
    auto it = std::find_if(mGameObjects.begin(), mGameObjects.end(),
        [gameObject](const std::unique_ptr<GameObject>& ptr) { return ptr.get() == gameObject; });
    if (it != mGameObjects.end())
    {
        mGameObjects.erase(it);
    }
}

void PhysicsWorld::logBodyCreation(const b2BodyDef& bodyDef, const char* source) const
{
    std::cout << "Creating body from " << source << ":" << std::endl;
    std::cout << "  Position: (" << bodyDef.position.x << ", " << bodyDef.position.y << ")" << std::endl;
    std::cout << "  Type: " << (bodyDef.type == b2_staticBody ? "Static" :
        bodyDef.type == b2_dynamicBody ? "Dynamic" : "Kinematic") << std::endl;
    std::cout << "  Angle: " << bodyDef.angle << std::endl;
}

b2Joint* PhysicsWorld::createJoint(const b2JointDef& jointDef)
{
    return mWorld->CreateJoint(&jointDef);
}

void PhysicsWorld::destroyJoint(b2Joint* joint)
{
    mWorld->DestroyJoint(joint);
}

void PhysicsWorld::BeginContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    b2Body* bodyA = fixtureA->GetBody();
    b2Body* bodyB = fixtureB->GetBody();

    GameObject* objectA = nullptr;
    GameObject* objectB = nullptr;

    if (bodyA)
    {
        uintptr_t userDataA = bodyA->GetUserData().pointer;
        if (userDataA != 0 && userDataA != uintptr_t(-1))
        {
            objectA = reinterpret_cast<GameObject*>(userDataA);
        }
    }

    if (bodyB)
    {
        uintptr_t userDataB = bodyB->GetUserData().pointer;
        if (userDataB != 0 && userDataB != uintptr_t(-1))
        {
            objectB = reinterpret_cast<GameObject*>(userDataB);
        }
    }

    if (objectA)
    {
        std::cout << "Collision detected: " << typeid(*objectA).name() << std::endl;
        objectA->onCollision(objectB);
    }

    if (objectB)
    {
        std::cout << "Collision detected: " << typeid(*objectB).name() << std::endl;
        objectB->onCollision(objectA);
    }
}

void PhysicsWorld::EndContact(b2Contact* contact)
{
    // End of contact
}

void PhysicsWorld::applyExplosionForce(const b2Vec2& center, float radius, float force)
{
    b2AABB aabb;
    aabb.lowerBound = center - b2Vec2(radius, radius);
    aabb.upperBound = center + b2Vec2(radius, radius);

    queryAABB(aabb, [&](b2Fixture* fixture) {
        b2Body* body = fixture->GetBody();
        b2Vec2 bodyCenter = body->GetWorldCenter();
        b2Vec2 direction = bodyCenter - center;
        float distance = direction.Normalize();

        if (distance <= radius)
        {
            float intensity = (1 - distance / radius) * force;
            body->ApplyLinearImpulse(intensity * direction, bodyCenter, true);

            GameObject* gameObject = reinterpret_cast<GameObject*>(body->GetUserData().pointer);
            if (gameObject)
            {
                if (auto* enemy = dynamic_cast<Enemy*>(gameObject))
                {
                    enemy->damage(intensity);
                }
                else if (auto* block = dynamic_cast<Block*>(gameObject))
                {
                    block->damage(intensity);
                }
            }
        }

        return true; // Continue querying
        });
}

void PhysicsWorld::addProjectile(std::unique_ptr<Projectile> projectile)
{
    if (projectile)
    {
        mGameObjects.push_back(std::move(projectile));
    }
}

void PhysicsWorld::applyBounceEffect(Projectile* projectile)
{
    if (!projectile) return;

    b2Body* body = projectile->getPhysicsBody();
    b2Vec2 velocity = body->GetLinearVelocity();

    // Increase velocity slightly with each bounce
    const float BOUNCE_FACTOR = 1.1f;
    body->SetLinearVelocity(BOUNCE_FACTOR * velocity);
}

void PhysicsWorld::cleanupMarkedObjects()
{
    mGameObjects.erase(
        std::remove_if(mGameObjects.begin(), mGameObjects.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                return obj->isMarkedForDeletion();
            }),
        mGameObjects.end());
}

void PhysicsWorld::queryAABB(const b2AABB& aabb, std::function<bool(b2Fixture*)> callback)
{
    class QueryCallback : public b2QueryCallback
    {
    public:
        QueryCallback(std::function<bool(b2Fixture*)> callback) : m_callback(callback) {}

        bool ReportFixture(b2Fixture* fixture) override
        {
            return m_callback(fixture);
        }

    private:
        std::function<bool(b2Fixture*)> m_callback;
    };

    QueryCallback query(callback);
    mWorld->QueryAABB(&query, aabb);
}

void PhysicsWorld::removeMarkedBodies()
{
    std::vector<b2Body*> bodiesToDestroy;

    for (b2Body* body = mWorld->GetBodyList(); body; )
    {
        b2Body* nextBody = body->GetNext(); // Store next body before potential destruction

        GameObject* obj = reinterpret_cast<GameObject*>(body->GetUserData().pointer);
        if (obj && obj->isMarkedForDeletion())
        {
            bodiesToDestroy.push_back(body);
        }

        body = nextBody;
    }

    for (b2Body* body : bodiesToDestroy)
    {
        mWorld->DestroyBody(body);
    }
}


void PhysicsWorld::createSplitProjectiles(Projectile* originalProjectile)
{
    if (!originalProjectile || !mWindow) return;

    b2Body* body = originalProjectile->getPhysicsBody();
    b2Vec2 position = body->GetPosition();
    b2Vec2 velocity = body->GetLinearVelocity();
    float speed = velocity.Length();

    const float SPLIT_ANGLE = 30.0f * b2_pi / 180.0f; // 30 degrees in radians

    for (int i = -1; i <= 1; i += 2) // Create two new projectiles
    {
        float angle = atan2(velocity.y, velocity.x) + i * SPLIT_ANGLE;
        b2Vec2 newVelocity(speed * cos(angle), speed * sin(angle));

        // Create a new projectile
        auto newProjectile = std::make_unique<Projectile>(*this, *originalProjectile->getTexture(), Projectile::Type::Standard, mWindow);
        newProjectile->setPosition(position.x * SCALE, position.y * SCALE);
        newProjectile->getPhysicsBody()->SetLinearVelocity(newVelocity);
        newProjectile->setLaunched(true);

        addProjectile(std::move(newProjectile));
    }
}