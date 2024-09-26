#include "joint_object.h"
#include "physics_world.h"

JointObject::JointObject(PhysicsWorld& world, const sf::Texture& texture, JointType type)
    : GameObject(), mJointType(type), mJoint(nullptr)
{
    setTexture(texture);
}

JointObject::~JointObject()
{
    if (mJoint && mPhysicsBody)
    {
        mPhysicsBody->GetWorld()->DestroyJoint(mJoint);
    }
}

void JointObject::update(sf::Time deltaTime)
{
    // Update position and rotation based on physics simulation
    if (mPhysicsBody)
    {
        b2Vec2 position = mPhysicsBody->GetPosition();
        float angle = mPhysicsBody->GetAngle();
        setPosition(position.x * PhysicsWorld::SCALE, position.y * PhysicsWorld::SCALE);
        setRotation(angle * 180.f / b2_pi);
    }
}

void JointObject::render(sf::RenderWindow& window)
{
    window.draw(mSprite);
}

void JointObject::onCollision(GameObject* other)
{
}

void JointObject::createJoint(b2Body* bodyA, b2Body* bodyB)
{
    switch (mJointType)
    {
    case JointType::Revolute:
        createRevoluteJoint(bodyA, bodyB);
        break;
    case JointType::Distance:
        createDistanceJoint(bodyA, bodyB);
        break;
    case JointType::Prismatic:
        createPrismaticJoint(bodyA, bodyB);
        break;
    case JointType::Pulley:
        createPulleyJoint(bodyA, bodyB);
        break;
    case JointType::Wheel:
        createWheelJoint(bodyA, bodyB);
        break;
    }
}

void JointObject::createRevoluteJoint(b2Body* bodyA, b2Body* bodyB)
{
    b2RevoluteJointDef jointDef;
    jointDef.bodyA = bodyA;
    jointDef.bodyB = bodyB;
    jointDef.localAnchorA.Set(0, 0);
    jointDef.localAnchorB.Set(0, 0);
    jointDef.enableLimit = true;
    jointDef.lowerAngle = -0.25f * b2_pi;
    jointDef.upperAngle = 0.25f * b2_pi;

    mJoint = mPhysicsBody->GetWorld()->CreateJoint(&jointDef);
}

void JointObject::createDistanceJoint(b2Body* bodyA, b2Body* bodyB)
{
    b2DistanceJointDef jointDef;
    jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldCenter(), bodyB->GetWorldCenter());
    jointDef.collideConnected = true;

    float stiffness = 1000.0f;
    float damping = 0.5f;     
    jointDef.stiffness = stiffness;
    jointDef.damping = damping;

    mJoint = mPhysicsBody->GetWorld()->CreateJoint(&jointDef);
}

void JointObject::createPrismaticJoint(b2Body* bodyA, b2Body* bodyB)
{
    b2PrismaticJointDef jointDef;
    b2Vec2 worldAxis(1.0f, 0.0f);
    jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldCenter(), worldAxis);
    jointDef.lowerTranslation = -5.0f;
    jointDef.upperTranslation = 5.0f;
    jointDef.enableLimit = true;
    jointDef.maxMotorForce = 1.0f;
    jointDef.motorSpeed = 0.0f;
    jointDef.enableMotor = true;

    mJoint = mPhysicsBody->GetWorld()->CreateJoint(&jointDef);
}

void JointObject::createPulleyJoint(b2Body* bodyA, b2Body* bodyB)
{
    b2PulleyJointDef jointDef;
    b2Vec2 anchorA = bodyA->GetWorldCenter();
    b2Vec2 anchorB = bodyB->GetWorldCenter();
    b2Vec2 groundAnchorA = anchorA + b2Vec2(0, -10);
    b2Vec2 groundAnchorB = anchorB + b2Vec2(0, -10);
    float ratio = 1.0f;

    jointDef.Initialize(bodyA, bodyB, groundAnchorA, groundAnchorB, anchorA, anchorB, ratio);

    mJoint = mPhysicsBody->GetWorld()->CreateJoint(&jointDef);
}

void JointObject::createWheelJoint(b2Body* bodyA, b2Body* bodyB)
{
    b2WheelJointDef jointDef;
    b2Vec2 axis(0.0f, 1.0f);
    jointDef.Initialize(bodyA, bodyB, bodyB->GetWorldCenter(), axis);
    jointDef.motorSpeed = 0.0f;
    jointDef.maxMotorTorque = 10.0f;
    jointDef.enableMotor = true;

    float stiffness = 8.0f;
    float damping = 0.7f;
    jointDef.stiffness = stiffness;
    jointDef.damping = damping;

    mJoint = mPhysicsBody->GetWorld()->CreateJoint(&jointDef);
}