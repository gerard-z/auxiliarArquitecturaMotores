#include "MonaEngine.hpp"
#include "Rendering/DiffuseFlatMaterial.hpp"

class Ball : public Mona::GameObject {
public:
	Ball(Mona::TransformHandle transform);

	~Ball();

	virtual void UserStartUp(Mona::World& world) noexcept;

	// Reaccionar a la colisión de la bola
	void OnCollisionBall(Mona::World& world, glm::vec3 collision_pos, glm::vec3 other_normal);

	// Revisa la colisión con el paddle
	void checkPaddleCollision(Mona::World& world, Mona::TransformHandle paddleTransform);
	
	virtual void UserUpdate(Mona::World& world, float timeStep) noexcept;

	float getBallRadius() const noexcept;

	Mona::TransformHandle GetTransform() const noexcept;

private:
	Mona::TransformHandle m_transform; // paddle transform
	Mona::TransformHandle m_ballTransform;
	std::shared_ptr<Mona::AudioClip> m_ballBounceSound;
	glm::vec3 m_ballVelocity;
	float m_ballRadius;
	float time_since_last_collision = 0.0f; // tiempo desde la última colisión con el paddle
};

