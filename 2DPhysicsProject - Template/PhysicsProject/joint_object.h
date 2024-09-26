#ifndef JOINT_OBJECT_H
#define JOINT_OBJECT_H

#include "box2d/box2d.h"
#include "game_object.h"

class JointObject : public GameObject
{
public:
    enum class JointType
    {
        Revolute,
        Distance,
        Prismatic,
        Pulley,
        Wheel
    };

    JointObject(PhysicsWorld& world, const sf::Texture& texture, JointType type);
    ~JointObject();

    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;

    void createJoint(b2Body* bodyA, b2Body* bodyB);

private:
    JointType mJointType;
    b2Joint* mJoint;

    void createRevoluteJoint(b2Body* bodyA, b2Body* bodyB);
    void createDistanceJoint(b2Body* bodyA, b2Body* bodyB);
    void createPrismaticJoint(b2Body* bodyA, b2Body* bodyB);
    void createPulleyJoint(b2Body* bodyA, b2Body* bodyB);
    void createWheelJoint(b2Body* bodyA, b2Body* bodyB);
};

#endif