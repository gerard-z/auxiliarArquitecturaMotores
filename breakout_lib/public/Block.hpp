#include "MonaEngine.hpp"
#include "Rendering/DiffuseFlatMaterial.hpp"
#include "Ball.hpp"

class Block : public Mona::GameObject {
public:
	Block(glm::vec3 init_position, glm::vec3 block_scale, std::shared_ptr<Mona::DiffuseFlatMaterial> block_material,
        Mona::TransformHandle ball_transform, float ball_radius, Mona::GameObjectHandle<Ball> ball);

	~Block();

	virtual void UserStartUp(Mona::World& world) noexcept;

	// Reaccionar a la colisión con la bola
	void OnCollisionWithBall(Mona::World& world);

	// Revisa la colisión con el paddle
	void checkBallCollision(Mona::World& world);

    void UserUpdate(Mona::World& world, float timeStep) noexcept;

private:
	Mona::GameObjectHandle<Ball> ball;
	Mona::TransformHandle ball_transform;
	float ball_radius;
	glm::vec3 position;
	glm::vec3 scale;
	std::shared_ptr<Mona::DiffuseFlatMaterial> material;
	std::shared_ptr<Mona::AudioClip> m_blockBreakingSound;
};