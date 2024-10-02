#include "MonaEngine.hpp"
#include "Rendering/DiffuseFlatMaterial.hpp"

class Paddle : public Mona::GameObject {
public:
	Paddle(float velocity);

	~Paddle();

	virtual void UserStartUp(Mona::World& world) noexcept;

	virtual void UserUpdate(Mona::World& world, float timeStep) noexcept;

	Mona::TransformHandle GetPaddleTransform() const noexcept;

private:
	Mona::TransformHandle m_transform;
	float m_paddleVelocity;
};