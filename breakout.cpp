#include "MonaEngine.hpp"
#include "Rendering/DiffuseFlatMaterial.hpp"
#include "Block.hpp"
#include "Paddle.hpp"

void CreateBasicCameraWithMusicAndLight(Mona::World& world)
{
	auto camera = world.CreateGameObject<Mona::GameObject>();
	auto transform = world.AddComponent<Mona::TransformComponent>(camera, glm::vec3(0.0f, 12.0f, 40.0f));
	transform->Rotate(glm::vec3(-1.0f, 0.0f, 0.0f), 1.57f);
	world.SetMainCamera(world.AddComponent<Mona::CameraComponent>(camera));
	world.SetAudioListenerTransform(transform);
	auto& audioClipManager = Mona::AudioClipManager::GetInstance();
	Mona::Config& config = Mona::Config::GetInstance();
	auto audioClipPtr = audioClipManager.LoadAudioClip(config.getPathOfApplicationAsset("audio/mewmew.wav"));
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
				float y = 2.0f * j + 15.0f;
				auto block = world.CreateGameObject<Block>(glm::vec3(x, y, 0.0f), glm::vec3(1.8f, 0.5f, 0.5f), materials[j+2], ball->GetTransform(), ball->getBallRadius(), ball);
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
};
int main()
{
	Breakout breakout;
	Mona::Engine engine(breakout);
	engine.StartMainLoop();
}