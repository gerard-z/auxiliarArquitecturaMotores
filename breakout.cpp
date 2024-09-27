#include "MonaEngine.hpp"
#include "Rendering/DiffuseFlatMaterial.hpp"

void CreateBasicCameraWithMusicAndLight(Mona::World& world)
{
	auto camera = world.CreateGameObject<Mona::GameObject>();
	auto transform = world.AddComponent<Mona::TransformComponent>(camera, glm::vec3(0.0f, 12.0f, 40.0f));
	transform->Rotate(glm::vec3(-1.0f, 0.0f, 0.0f), 1.57f);
	world.SetMainCamera(world.AddComponent<Mona::CameraComponent>(camera));
	world.SetAudioListenerTransform(transform);
	auto& audioClipManager = Mona::AudioClipManager::GetInstance();
	auto audioClipPtr = audioClipManager.LoadAudioClip(Mona::SourceDirectoryData::SourcePath("Assets/AudioFiles/music.wav"));
	auto audioSource = world.AddComponent<Mona::AudioSourceComponent>(camera, audioClipPtr);
	audioSource->SetIsLooping(true);
	audioSource->SetVolume(0.3f);
	audioSource->Play();

	world.AddComponent<Mona::DirectionalLightComponent>(camera, glm::vec3(1.0f));

}

void CreateWall(Mona::World& world,
	const glm::vec3& position,
	const glm::vec3& scale,
	std::shared_ptr<Mona::Material> wallMaterial) {
	auto& meshManager = Mona::MeshManager::GetInstance();
	auto wall = world.CreateGameObject<Mona::GameObject>();
	world.AddComponent<Mona::TransformComponent>(wall, position, glm::fquat(1.0f, 0.0f, 0.0f, 0.0f), scale);
	world.AddComponent<Mona::StaticMeshComponent>(wall, meshManager.LoadMesh(Mona::Mesh::PrimitiveType::Cube), wallMaterial);
}


class Ball : public Mona::GameObject {
public:
	Ball(Mona::TransformHandle transform) : m_transform(transform) {}
	~Ball() = default;
	virtual void UserStartUp(Mona::World& world) noexcept {
		auto& audioClipManager = Mona::AudioClipManager::GetInstance();
		m_ballBounceSound = audioClipManager.LoadAudioClip(Mona::SourceDirectoryData::SourcePath("Assets/AudioFiles/ballBounce.wav"));
		auto& meshManager = Mona::MeshManager::GetInstance();
		// Crear la bola, su material y su forma.
		auto ball = world.CreateGameObject<Mona::GameObject>();
		m_ballRadius = 0.5f;
		m_ballTransform = world.AddComponent<Mona::TransformComponent>(ball);
		m_ballTransform->SetRotation(m_transform->GetLocalRotation());
		m_ballTransform->SetTranslation(m_transform->GetLocalTranslation() + glm::vec3(0.0f, 2.0f, 0.0f));
		m_ballTransform->SetScale(glm::vec3(m_ballRadius));
		auto ballMaterial = std::static_pointer_cast<Mona::DiffuseFlatMaterial>(world.CreateMaterial(Mona::MaterialType::DiffuseFlat));
		ballMaterial->SetDiffuseColor(glm::vec3(0.75f, 0.3f, 0.3f));
		world.AddComponent<Mona::StaticMeshComponent>(ball, meshManager.LoadMesh(Mona::Mesh::PrimitiveType::Sphere), ballMaterial);

		// Vector velocidad de la bola
		m_ballVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	// Reaccionar a la colisión de la bola
	void OnCollisionBall(Mona::World& world, glm::vec3 collision_pos, glm::vec3 other_normal) {
		world.PlayAudioClip3D(m_ballBounceSound, m_ballTransform->GetLocalTranslation(), 0.3f);
		m_ballTransform->SetTranslation(collision_pos + other_normal * m_ballRadius);
		m_ballVelocity = glm::reflect(m_ballVelocity, other_normal);
	}

	// Revisa la colisión con el paddle
	void checkPaddleCollision(Mona::World& world, Mona::TransformHandle paddleTransform) {
		if (time_since_last_collision < 0.2f) { // Evitar interacciones múltiples en la misma colisión
			return;
		}

		auto paddlePos = paddleTransform->GetLocalTranslation();
		auto paddleScale = paddleTransform->GetLocalScale();
		auto ballPos = m_ballTransform->GetLocalTranslation();
		// Primero revisamos con un bounding box, si no colisiona con el bounding box no colisiona con el paddle
		if (ballPos.x + m_ballRadius < paddlePos.x - paddleScale.x || ballPos.x - m_ballRadius > paddlePos.x + paddleScale.x) {
			return;
		}
		if (ballPos.y + m_ballRadius < paddlePos.y - paddleScale.y || ballPos.y - m_ballRadius > paddlePos.y + paddleScale.y) {
			return;
		}
		if (ballPos.z + m_ballRadius < paddlePos.z - paddleScale.z || ballPos.z - m_ballRadius > paddlePos.z + paddleScale.z) { // no debería pasar
			return;
		}
		// Si colisiona con el bounding box, revisamos si el punto más cercano del paddle a la bola está dentro de la bola
		float closestPoint_x = glm::clamp(ballPos.x, paddlePos.x - paddleScale.x, paddlePos.x + paddleScale.x);
		float closestPoint_y = glm::clamp(ballPos.y, paddlePos.y - paddleScale.y, paddlePos.y + paddleScale.y);
		float closestPoint_z = glm::clamp(ballPos.z, paddlePos.z - paddleScale.z, paddlePos.z + paddleScale.z);

		float distance = glm::distance(glm::vec3(closestPoint_x, closestPoint_y, closestPoint_z), glm::vec3(ballPos.x, ballPos.y, ballPos.z));

		if (distance > m_ballRadius) {
			return;
		}

		// Si el punto más cercano está dentro de la bola, entonces la bola colisionó con el paddle
		time_since_last_collision = 0.0f;
		glm::vec3 normal = glm::normalize(glm::vec3(ballPos.x - closestPoint_x, ballPos.y - closestPoint_y, ballPos.z - closestPoint_z));
		OnCollisionBall(world, glm::vec3(closestPoint_x, closestPoint_y, closestPoint_z), normal);
	}
	

	virtual void UserUpdate(Mona::World& world, float timeStep) noexcept {
		auto& input = world.GetInput();

		// Lanzar la bola para empezar el juego
		if (input.IsMouseButtonPressed(MONA_MOUSE_BUTTON_1) && m_ballVelocity == glm::vec3(0.0f, 0.0f, 0.0f)) {
			m_ballVelocity = glm::vec3(0.0f, 15.0f, 0.0f);
			
		}

		// Actualizar la posición de la bola
		m_ballTransform->SetTranslation(m_ballTransform->GetLocalTranslation() + m_ballVelocity * timeStep);

		// Colisiones con el paddle
		checkPaddleCollision(world, m_transform);
		if (time_since_last_collision < 0.5f) {
			time_since_last_collision += timeStep;
		}

		// Colisiones con las paredes
		if (m_ballTransform->GetLocalTranslation().x < -17.0f) {
			OnCollisionBall(world, glm::vec3(-17.0f, m_ballTransform->GetLocalTranslation().y, m_ballTransform->GetLocalTranslation().z), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (m_ballTransform->GetLocalTranslation().x > 17.0f) {
			OnCollisionBall(world, glm::vec3(17.0f, m_ballTransform->GetLocalTranslation().y, m_ballTransform->GetLocalTranslation().z), glm::vec3(-1.0f, 0.0f, 0.0f));
		}
		else if (m_ballTransform->GetLocalTranslation().y < -4.0f) {
			OnCollisionBall(world, glm::vec3(m_ballTransform->GetLocalTranslation().x, -4.0f, m_ballTransform->GetLocalTranslation().z), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (m_ballTransform->GetLocalTranslation().y > 25.0f) {
			OnCollisionBall(world, glm::vec3(m_ballTransform->GetLocalTranslation().x, 25.0f, m_ballTransform->GetLocalTranslation().z), glm::vec3(0.0f, -1.0f, 0.0f));
		}

	}

private:
	Mona::TransformHandle m_transform; // paddle transform
	Mona::TransformHandle m_ballTransform;
	std::shared_ptr<Mona::AudioClip> m_ballBounceSound;
	glm::vec3 m_ballVelocity;
	float m_ballRadius;
	float time_since_last_collision = 0.0f; // tiempo desde la última colisión con el paddle
};
		


class Paddle : public Mona::GameObject {
public:
	Paddle(float velocity) : m_paddleVelocity(velocity) {}
	~Paddle() = default;
	virtual void UserStartUp(Mona::World& world) noexcept {
		m_transform = world.AddComponent<Mona::TransformComponent>(*this);
		glm::vec3 paddleScale(2.0f, 0.5f, 0.5f);
		m_transform->Scale(paddleScale);
		auto paddleMaterial = std::static_pointer_cast<Mona::DiffuseFlatMaterial>(world.CreateMaterial(Mona::MaterialType::DiffuseFlat));
		paddleMaterial->SetDiffuseColor(glm::vec3(0.3f, 0.3f, 0.75f));
		auto& meshManager = Mona::MeshManager::GetInstance();
		world.AddComponent<Mona::StaticMeshComponent>(*this, meshManager.LoadMesh(Mona::Mesh::PrimitiveType::Cube), paddleMaterial);
		Mona::BoxShapeInformation boxInfo(paddleScale);
		Mona::RigidBodyHandle rb = world.AddComponent<Mona::RigidBodyComponent>(*this, boxInfo, Mona::RigidBodyType::KinematicBody);
		rb->SetFriction(0.0f);
		rb->SetRestitution(1.0f);
	}

	virtual void UserUpdate(Mona::World& world, float timeStep) noexcept {
		auto& input = world.GetInput();
		if (input.IsKeyPressed(MONA_KEY_A))
		{
			m_transform->Translate(glm::vec3(-m_paddleVelocity * timeStep, 0.0f, 0.0f));
		}
		else if (input.IsKeyPressed(MONA_KEY_D))
		{
			m_transform->Translate(glm::vec3(m_paddleVelocity * timeStep, 0.0f, 0.0f));
		}

		if (input.IsKeyPressed(MONA_KEY_ESCAPE)) {
			exit(EXIT_SUCCESS);
		}

		if (m_transform->GetLocalTranslation().x < -16.0f) {
			m_transform->SetTranslation(glm::vec3(-16.0f, 0, 0));
		}
		else if (m_transform->GetLocalTranslation().x > 16.0f) {
			m_transform->SetTranslation(glm::vec3(16.0f, 0, 0));
		}

	}

	Mona::TransformHandle GetPaddleTransform() const noexcept {
		return m_transform;
	}

private:
	Mona::TransformHandle m_transform;
	float m_paddleVelocity;
};



class Breakout : public Mona::Application {
public:
	Breakout() = default;
	~Breakout() = default;
	virtual void UserStartUp(Mona::World & world) noexcept override {
		world.SetGravity(glm::vec3(0.0f,0.0f,0.0f));
		world.SetAmbientLight(glm::vec3(0.3f));
		CreateBasicCameraWithMusicAndLight(world);
		auto paddle = world.CreateGameObject<Paddle>(20.0f);
		auto ball = world.CreateGameObject<Ball>(paddle->GetPaddleTransform());
		

		//Crear el los bloques destructibles del nivel
		glm::vec3 blockScale(1.8f, 0.5f, 0.5f);
		auto& audioClipManager = Mona::AudioClipManager::GetInstance();
		m_blockBreakingSound = audioClipManager.LoadAudioClip(Mona::SourceDirectoryData::SourcePath("Assets/AudioFiles/boxBreaking.wav"));
		auto& meshManager = Mona::MeshManager::GetInstance();
		Mona::BoxShapeInformation boxInfo(blockScale);

		std::shared_ptr<Mona::DiffuseFlatMaterial> materials[5];
		glm::vec3 colors[5] = { glm::vec3(0.0f,0.0f,1.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(1.0f,1.0f,0.0f), glm::vec3(1.0f,0.0f,0.0f), glm::vec3(1.0f,0.0f,1.0f) };
		for (int i=0; i<5; i++) {
			auto blockMaterial = world.CreateMaterial(Mona::MaterialType::DiffuseFlat);
			auto blockMaterialPtr = std::static_pointer_cast<Mona::DiffuseFlatMaterial>(blockMaterial);
			blockMaterialPtr->SetDiffuseColor(colors[i]);
			materials[i] = blockMaterialPtr;
		}

		for (int i = -2; i < 3; i++) {
			float x = 4.0f * i;
			for (int j = -2; j < 3; j++)
			{
				float y = 2.0f * j;
				auto block = world.CreateGameObject<Mona::GameObject>();
				auto transform = world.AddComponent<Mona::TransformComponent>(block, glm::vec3( x, 15.0f + y, 0.0f));
				transform->Scale(blockScale);
				world.AddComponent<Mona::StaticMeshComponent>(block, meshManager.LoadMesh(Mona::Mesh::PrimitiveType::Cube), materials[j+2]);
				Mona::RigidBodyHandle rb =world.AddComponent<Mona::RigidBodyComponent>(block, boxInfo, Mona::RigidBodyType::StaticBody, 1.0f);
				rb->SetRestitution(1.0f);
				rb->SetFriction(0.0f);
				auto callback = [block, blockSound = m_blockBreakingSound](Mona::World& world, Mona::RigidBodyHandle& otherRigidBody, bool isSwaped, Mona::CollisionInformation& colInfo) mutable {
					world.PlayAudioClip2D(blockSound, 1.0f, 1.0f);
					world.DestroyGameObject(block);
				};

				rb->SetStartCollisionCallback(callback);
				
			}
		}

		//Crear las paredes
		auto wallMaterial = std::static_pointer_cast<Mona::DiffuseFlatMaterial>(world.CreateMaterial(Mona::MaterialType::DiffuseFlat));
		wallMaterial->SetDiffuseColor(glm::vec3(0.15f, 0.15f, 0.15f));


		CreateWall(world, glm::vec3(0.0f, 26.0f, 0.0f), glm::vec3(18.0f, 1.0f, 1.0f), wallMaterial);
		CreateWall(world, glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(18.0f, 1.0f, 1.0f), wallMaterial);

		glm::vec3 sideWallScale(1.0f, 27.0f, 1.0f);
		float sideWallOffset = 19.0f;
		CreateWall(world, glm::vec3(-sideWallOffset, 0.0f, 0.0f), sideWallScale, wallMaterial);
		CreateWall(world, glm::vec3(sideWallOffset, 0.0f, 0.0f), sideWallScale, wallMaterial);

	}

	virtual void UserShutDown(Mona::World& world) noexcept override {
	}
	virtual void UserUpdate(Mona::World & world, float timeStep) noexcept override {
	}
	std::shared_ptr<Mona::AudioClip> m_blockBreakingSound;
};
int main()
{
	Breakout breakout;
	Mona::Engine engine(breakout);
	engine.StartMainLoop();
}